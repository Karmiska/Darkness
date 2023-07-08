#pragma once

#include "containers/memory.h"
#include "containers/vector.h"
#include "engine/primitives/Vector4.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/graphics/CommandList.h"
#include "tools/image/ImageIf.h"
#include "shaders/core/ui/DrawRectangle.h"
#include "shaders/core/ui/DrawRectangles.h"
#include "shaders/core/ui/DrawImage.h"
#include "shaders/core/ui/DrawImageUint.h"
#include "shaders/core/ui/DrawText.h"
#include "shaders/core/ui/DrawTextSimple.h"
#include "containers/unordered_map.h"
#include "tools/image/ImageIf.h"
#include "engine/font/Font.h"
#include "tools/RingBuffer.h"
#include "engine/graphics/Rect.h"

#include <stack>

namespace engine
{
	class Device;
	class TextureSRV;
	class RenderSetup;
}

namespace ui
{
	constexpr unsigned MaxGlyphsInFlight = 100000;
	constexpr unsigned MaxRectanglesInFlight = 500000;

	class DrawCommandBuffer
	{
	public:
		DrawCommandBuffer(engine::Device& device);
		~DrawCommandBuffer();

		void drawRectangle(int x, int y, int width, int height, const engine::Vector4f& color);
		void drawImage(int x, int y, int width, int height, engine::shared_ptr<engine::image::ImageIf> image);
		void drawImage(int x, int y, int width, int height, engine::TextureSRV image);
		void drawText(int x, int y, int width, int height, const engine::string& text);
		void executeCommandList(engine::RenderSetup* renderer, engine::CommandList&& cmdList);

		bool isOpen() const { return m_open; }
		void open() { m_open = true; }
		void close() { m_open = false; }

		void reset();

		struct RenderCommandList
		{
			engine::CommandList cmdList;
			engine::RenderSetup* renderer;
		};
		engine::vector<RenderCommandList> recordCommands(engine::RenderSetup* renderSetup, engine::TextureRTV rtv);

		struct GlobalTransform
		{
			int objectX;
			int objectY;
			int objectWidth;
			int objectHeight;

			int clipX;
			int clipY;
			int clipWidth;
			int clipHeight;
		};
		void pushTransform(const GlobalTransform& transform);
		GlobalTransform popTransform();
		GlobalTransform currentTransform() const;
	private:
		engine::Device& m_device;
		bool m_open;
		enum class CommandType : unsigned char
		{
			Rectangle,
			Image,
			Text,
			CommandList
		};

		class PacketBase
		{
		public:
			virtual ~PacketBase() {};
		};

		class PacketRectangle : public PacketBase
		{
		public:
			short x;
			short y;
			unsigned short width;
			unsigned short height;
			float r;
			float g;
			float b;
			float a;
		};

		class PacketImage : public PacketBase
		{
		public:
			short x;
			short y;
			unsigned short width;
			unsigned short height;
			engine::image::ImageIf* image;
			engine::TextureSRV texture;
		};

		class PacketText : public PacketBase
		{
		public:
			short x;
			short y;
			unsigned short width;
			unsigned short height;
			const char* text;
		};

		class PacketCommandList : public PacketBase
		{
		public:
			engine::CommandList cmdList;
			engine::RenderSetup* renderer;
		};

		#pragma pack(push, 1)
		struct CommandPacket
		{
			CommandType type;
			engine::Rectangle scissor;

			PacketBase* data;
			/*union {
				PacketRectangle rectangle;
				PacketImage image;
				PacketText text;
				PacketCommandList commandList;
			};*/

			CommandPacket()
				: type{}
				, scissor{}
				, data{nullptr}
			{};
			~CommandPacket()
			{
				if (data)
					delete data;
			};

			CommandPacket(const CommandPacket& packet)
				: type{ packet.type }
				, scissor{ packet.scissor }
			{
				assignFrom(packet);
			}
			CommandPacket& operator=(const CommandPacket& packet)
			{
				type = packet.type;
				scissor = packet.scissor;
				assignFrom(packet);
			}
			CommandPacket(CommandPacket&& packet)
				: type{ packet.type }
				, scissor{ packet.scissor }
				, data{ nullptr }
			{
				std::swap(data, packet.data);
			}
			CommandPacket& operator=(CommandPacket&& packet)
			{
				std::swap(type, packet.type);
				std::swap(scissor, packet.scissor);
				std::swap(data, packet.data);
			}

		private:
			void assignFrom(const CommandPacket& packet)
			{
				if (packet.type == CommandType::Rectangle)
				{
					auto d = new PacketRectangle();
					*d = *static_cast<PacketRectangle*>(packet.data);
					data = d;

				}
				else if (packet.type == CommandType::Image)
				{
					auto d = new PacketImage();
					*d = *static_cast<PacketImage*>(packet.data);
					data = d;
				}
				else if (packet.type == CommandType::Text)
				{
					auto d = new PacketText();
					*d = *static_cast<PacketText*>(packet.data);
					data = d;
				}
				else if (packet.type == CommandType::CommandList)
				{
					auto d = new PacketCommandList();
					*d = *static_cast<PacketCommandList*>(packet.data);
					data = d;
				}
			}
		};
		#pragma pack(pop)

		struct RectangleItem
		{
			float2 position;
			float2 size;
			uint color;
			uint3 padding;
		};

		engine::vector<CommandPacket> m_commands;

		std::stack<GlobalTransform> m_transformStack;

		const char* allocateFromStringBuffer(const engine::string& text);
		engine::vector<char> m_stringBuffer;
		size_t m_stringBufferIndex;

		void recordRectangle(engine::CommandList& cmd, engine::TextureRTV rtv, const CommandPacket& packet);
		void recordImage(engine::CommandList& cmd, engine::TextureRTV rtv, const CommandPacket& packet);
		void recordText(engine::CommandList& cmd, engine::TextureRTV rtv, const CommandPacket& packet);
		void recordText(
			engine::CommandList& cmd, 
			engine::TextureRTV rtv, 
			const engine::vector<PacketText>& textPackets, 
			const engine::vector<engine::Rectangle>& scissors);

		engine::Pipeline<engine::shaders::DrawRectangle> m_drawRectangle;
		engine::Pipeline<engine::shaders::DrawRectangles> m_drawRectangles;
		engine::Pipeline<engine::shaders::DrawImage> m_drawImage;
		engine::Pipeline<engine::shaders::DrawImageUint> m_drawImageUint;
		engine::Pipeline<engine::shaders::DrawText> m_drawText;
		engine::Pipeline<engine::shaders::DrawTextSimple> m_drawTextSimple;

		engine::unordered_map<engine::image::ImageIf*, engine::TextureSRVOwner> m_textures;

		engine::shared_ptr<engine::Font> m_font;
		engine::BufferOwner m_uploadBuffer;
		engine::BufferSRVOwner m_uploadBufferSRV;
		engine::Font::GlyphRenderNodeData* m_uploadPtr;
		tools::RingBuffer m_uploadRing;

		engine::BufferOwner m_glyphPositionsUploadBuffer;
		engine::BufferSRVOwner m_glyphPositionsUploadBufferSRV;
		engine::Vector2f* m_glyphPositionsUploadPtr;
		tools::RingBuffer m_glyphPositionsUploadRing;

		engine::BufferOwner m_rectanglesUploadBuffer;
		engine::BufferSRVOwner m_rectanglesUploadBufferSRV;
		RectangleItem* m_rectanglesUploadPtr;
		tools::RingBuffer m_rectanglesUploadRing;

		engine::TextureRTVOwner m_glyphScissorMaskRTV;
		engine::TextureSRVOwner m_glyphScissorMaskSRV;
		void resizeGlyphScissorMask(uint32_t width, uint32_t height);
	};
}
