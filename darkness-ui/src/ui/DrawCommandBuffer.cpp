#include "ui/DrawCommandBuffer.h"
#include "ui/UiRect.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/RenderSetup.h"
#include "engine/font/FontManager.h"
#include "engine/font/Font.h"
#include "engine/graphics/Rect.h"
#include "tools/ByteRange.h"
#include <algorithm>

namespace ui
{
	DrawCommandBuffer::DrawCommandBuffer(engine::Device& device)
		: m_device{ device }
		, m_open{ false }
		, m_drawRectangle{ device.createPipeline<engine::shaders::DrawRectangle>() }
		, m_drawRectangles{ device.createPipeline<engine::shaders::DrawRectangles>() }
		, m_drawImage{ device.createPipeline<engine::shaders::DrawImage>() }
		, m_drawImageUint{ device.createPipeline<engine::shaders::DrawImageUint>() }
		, m_drawText{ device.createPipeline<engine::shaders::DrawText>() }
		, m_drawTextSimple{ device.createPipeline<engine::shaders::DrawTextSimple>() }
		, m_font{ nullptr }
		, m_uploadRing{ tools::ByteRange{ 
			reinterpret_cast<uint8_t*>(0), 
			reinterpret_cast<uint8_t*>(MaxGlyphsInFlight) } }
		, m_glyphPositionsUploadRing{ tools::ByteRange{
			reinterpret_cast<uint8_t*>(0), 
			reinterpret_cast<uint8_t*>(MaxGlyphsInFlight) } }
		, m_rectanglesUploadRing{ tools::ByteRange{
			reinterpret_cast<uint8_t*>(0), 
			reinterpret_cast<uint8_t*>(MaxRectanglesInFlight) } }
	{
		m_stringBuffer.resize(655360);
		//m_font = m_device.fontManager().loadFont("C:\\work\\darkness\\darkness-editor-v2\\data\\Roboto\\Roboto-Regular.ttf");
		m_font = m_device.fontManager().loadFont("C:\\work\\darkness\\darkness-editor-v2\\data\\SegoeUI.ttf");
		

		engine::DepthStencilOpDescription front;
		front.StencilFailOp = engine::StencilOp::Keep;
		front.StencilDepthFailOp = engine::StencilOp::Incr;
		front.StencilPassOp = engine::StencilOp::Keep;
		front.StencilFunc = engine::ComparisonFunction::Always;

		engine::DepthStencilOpDescription back;
		back.StencilFailOp = engine::StencilOp::Keep;
		back.StencilDepthFailOp = engine::StencilOp::Decr;
		back.StencilPassOp = engine::StencilOp::Keep;
		back.StencilFunc = engine::ComparisonFunction::Always;

		m_drawRectangle.setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleStrip);
		m_drawRectangle.setRasterizerState(engine::RasterizerDescription().cullMode(engine::CullMode::None).fillMode(engine::FillMode::Solid)); // TODO
		m_drawRectangle.setDepthStencilState(engine::DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(engine::DepthWriteMask::All)
			.depthFunc(engine::ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

		m_drawRectangles.setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleList);
		m_drawRectangles.setRasterizerState(engine::RasterizerDescription().cullMode(engine::CullMode::None).fillMode(engine::FillMode::Solid)); // TODO
		m_drawRectangles.setDepthStencilState(engine::DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(engine::DepthWriteMask::All)
			.depthFunc(engine::ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

		m_drawImage.setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleStrip);
		m_drawImage.setRasterizerState(engine::RasterizerDescription().cullMode(engine::CullMode::None).fillMode(engine::FillMode::Solid)); // TODO
		m_drawImage.setDepthStencilState(engine::DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(engine::DepthWriteMask::All)
			.depthFunc(engine::ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
		m_drawImage.ps.imageSampler = device.createSampler(engine::SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(engine::TextureAddressMode::Clamp));

		m_drawImageUint.setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleStrip);
		m_drawImageUint.setRasterizerState(engine::RasterizerDescription().cullMode(engine::CullMode::None).fillMode(engine::FillMode::Solid)); // TODO
		m_drawImageUint.setDepthStencilState(engine::DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(engine::DepthWriteMask::All)
			.depthFunc(engine::ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

		m_drawText.setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleList);
		m_drawText.setRasterizerState(engine::RasterizerDescription().frontCounterClockwise(false)
			.cullMode(engine::CullMode::None)
			.fillMode(engine::FillMode::Solid)); // TODO
		m_drawText.setDepthStencilState(engine::DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(engine::DepthWriteMask::All)
			.depthFunc(engine::ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
		m_drawText.ps.imageSampler = device.createSampler(engine::SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(engine::TextureAddressMode::Clamp));
		m_drawText.setBlendState(engine::BlendDescription().renderTarget(
			0, engine::RenderTargetBlendDescription()
			.blendEnable(true)

			.srcBlend(engine::Blend::SrcAlpha)
			.dstBlend(engine::Blend::InvSrcAlpha)
			.blendOp(engine::BlendOperation::Add)

			.srcBlendAlpha(engine::Blend::InvSrcAlpha)
			.dstBlendAlpha(engine::Blend::Zero)
			.blendOpAlpha(engine::BlendOperation::Add)

			.renderTargetWriteMask(1 | 2 | 4 | 8)
		));

		m_drawTextSimple.setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleList);
		m_drawTextSimple.setRasterizerState(engine::RasterizerDescription().frontCounterClockwise(false)
			.cullMode(engine::CullMode::None)
			.fillMode(engine::FillMode::Solid)); // TODO
		m_drawTextSimple.setDepthStencilState(engine::DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(engine::DepthWriteMask::All)
			.depthFunc(engine::ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
		m_drawTextSimple.ps.imageSampler = device.createSampler(engine::SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(engine::TextureAddressMode::Clamp));
		m_drawTextSimple.setBlendState(engine::BlendDescription().renderTarget(
			0, engine::RenderTargetBlendDescription()
			.blendEnable(true)

			.srcBlend(engine::Blend::SrcAlpha)
			.dstBlend(engine::Blend::InvSrcAlpha)
			.blendOp(engine::BlendOperation::Add)

			.srcBlendAlpha(engine::Blend::InvSrcAlpha)
			.dstBlendAlpha(engine::Blend::Zero)
			.blendOpAlpha(engine::BlendOperation::Add)

			.renderTargetWriteMask(1 | 2 | 4 | 8)
		));

		m_uploadBuffer = m_device.createBuffer(engine::BufferDescription()
				.usage(engine::ResourceUsage::Upload)
				.elementSize(sizeof(engine::Font::GlyphRenderNodeData))
				.elements(MaxGlyphsInFlight)
				.structured(true)
				.name("Text UploadBuffer"));
		m_uploadBufferSRV = m_device.createBufferSRV(m_uploadBuffer);
		m_uploadPtr = reinterpret_cast<engine::Font::GlyphRenderNodeData*>(m_uploadBuffer.resource().map(m_device));

		m_glyphPositionsUploadBuffer = m_device.createBuffer(engine::BufferDescription()
			.usage(engine::ResourceUsage::Upload)
			.elementSize(sizeof(engine::Vector2f))
			.elements(MaxGlyphsInFlight)
			.structured(true)
			.name("Text position UploadBuffer"));
		m_glyphPositionsUploadBufferSRV = m_device.createBufferSRV(m_glyphPositionsUploadBuffer);
		m_glyphPositionsUploadPtr = reinterpret_cast<engine::Vector2f*>(m_glyphPositionsUploadBuffer.resource().map(m_device));

		m_rectanglesUploadBuffer = m_device.createBuffer(engine::BufferDescription()
			.usage(engine::ResourceUsage::Upload)
			.elementSize(sizeof(RectangleItem))
			.elements(MaxRectanglesInFlight)
			.structured(true)
			.name("Rectangles UploadBuffer"));
		m_rectanglesUploadBufferSRV = m_device.createBufferSRV(m_rectanglesUploadBuffer);
		m_rectanglesUploadPtr = reinterpret_cast<RectangleItem*>(m_rectanglesUploadBuffer.resource().map(m_device));
	}

	DrawCommandBuffer::~DrawCommandBuffer()
	{
		m_font = nullptr;
	}

	void DrawCommandBuffer::drawRectangle(int x, int y, int width, int height, const engine::Vector4f& color)
	{
		auto currentTrans = currentTransform();

		auto rectangle = new PacketRectangle();
		rectangle->x = static_cast<short>(currentTrans.objectX + x);
		rectangle->y = static_cast<short>(currentTrans.objectY + y);
		rectangle->width = static_cast<unsigned short>(width);
		rectangle->height = static_cast<unsigned short>(height);
		rectangle->r = color.x;
		rectangle->g = color.y;
		rectangle->b = color.z;
		rectangle->a = color.w;
		
		CommandPacket packet;
		packet.type = CommandType::Rectangle;
		packet.scissor = engine::Rectangle{ 
			currentTrans.clipX,
			currentTrans.clipY,
			currentTrans.clipX + currentTrans.clipWidth, 
			currentTrans.clipY + currentTrans.clipHeight };
		if (packet.scissor.top > packet.scissor.bottom)
			packet.scissor.bottom = packet.scissor.top;
		packet.data = rectangle;

		m_commands.emplace_back(std::move(packet));
	}

	void DrawCommandBuffer::drawImage(int x, int y, int width, int height, engine::shared_ptr<engine::image::ImageIf> image)
	{
		auto currentTrans = currentTransform();

		auto pimage = new PacketImage();
		pimage->x = static_cast<short>(currentTrans.objectX + x);
		pimage->y = static_cast<short>(currentTrans.objectY + y);
		pimage->width = static_cast<unsigned short>(width);
		pimage->height = static_cast<unsigned short>(height);
		pimage->image = image.get();
		
		CommandPacket packet;
		packet.type = CommandType::Image;
		packet.scissor = engine::Rectangle{
			currentTrans.clipX,
			currentTrans.clipY,
			currentTrans.clipX + currentTrans.clipWidth,
			currentTrans.clipY + currentTrans.clipHeight };
		packet.data = pimage;

		ASSERT(pimage->image != nullptr, "Can't draw nullptr image");

		m_commands.emplace_back(std::move(packet));
	}

	void DrawCommandBuffer::drawImage(int x, int y, int width, int height, engine::TextureSRV image)
	{
		auto currentTrans = currentTransform();

		auto pimage = new PacketImage();
		pimage->x = static_cast<short>(currentTrans.objectX + x);
		pimage->y = static_cast<short>(currentTrans.objectY + y);
		pimage->width = static_cast<unsigned short>(width);
		pimage->height = static_cast<unsigned short>(height);
		pimage->image = nullptr;
		pimage->texture = image;

		CommandPacket packet;
		packet.type = CommandType::Image;
		packet.scissor = engine::Rectangle{
			currentTrans.clipX,
			currentTrans.clipY,
			currentTrans.clipX + currentTrans.clipWidth,
			currentTrans.clipY + currentTrans.clipHeight };
		packet.data = pimage;

		ASSERT(pimage->texture.valid(), "Can't draw nullptr image");

		m_commands.emplace_back(std::move(packet));
	}

	void DrawCommandBuffer::drawText(int x, int y, int width, int height, const engine::string& text)
	{
		auto currentTrans = currentTransform();

		auto ptext = new PacketText();
		ptext->x = static_cast<short>(currentTrans.objectX + x);
		ptext->y = static_cast<short>(currentTrans.objectY + y);
		ptext->width = static_cast<unsigned short>(width);
		ptext->height = static_cast<unsigned short>(height);
		ptext->text = allocateFromStringBuffer(text);

		CommandPacket packet;
		packet.type = CommandType::Text;
		packet.scissor = engine::Rectangle{
			currentTrans.clipX,
			currentTrans.clipY,
			currentTrans.clipX + currentTrans.clipWidth,
			currentTrans.clipY + currentTrans.clipHeight };
		packet.data = ptext;

		m_commands.emplace_back(std::move(packet));
	}

	void DrawCommandBuffer::executeCommandList(engine::RenderSetup* renderer, engine::CommandList&& cmdList)
	{
		auto cmdListData = new PacketCommandList();
		cmdListData->cmdList = std::move(cmdList);
		cmdListData->renderer = renderer;

		CommandPacket packet;
		packet.type = CommandType::CommandList;
		packet.data = cmdListData;
		m_commands.emplace_back(std::move(packet));
	}

	void DrawCommandBuffer::reset()
	{
		m_stringBufferIndex = 0;
		m_commands.clear();
	}

	const char* DrawCommandBuffer::allocateFromStringBuffer(const engine::string& text)
	{
		size_t targetSize = text.size() + 1;
		size_t allocationSize = m_stringBufferIndex + targetSize;
		if (allocationSize > m_stringBuffer.size())
			m_stringBuffer.resize(std::max(allocationSize, m_stringBuffer.size() * 2));

		const char* result = &m_stringBuffer[m_stringBufferIndex];
		memcpy(&m_stringBuffer[m_stringBufferIndex], text.data(), targetSize - 1);
		m_stringBufferIndex += text.length() + 1;
		m_stringBuffer[m_stringBufferIndex-1] = '\0';
		return result;
	}

	engine::vector<DrawCommandBuffer::RenderCommandList> DrawCommandBuffer::recordCommands(engine::RenderSetup* renderSetup, engine::TextureRTV rtv)
	{
		engine::vector<RenderCommandList> lists;
		engine::CommandList currentList;
		bool ListInUse = false;

		auto TakeListInUse = [&]()
		{
			if (ListInUse)
				return;
			currentList = renderSetup->device().createCommandList("Frame");
			ListInUse = true;
		};

		auto EndList = [&]()
		{
			if (!ListInUse)
				return;
			RenderCommandList renderCommandList;
			renderCommandList.cmdList = std::move(currentList);
			renderCommandList.renderer = nullptr;
			lists.emplace_back(std::move(renderCommandList));
			ListInUse = false;
		};

		TakeListInUse();
		currentList.clearRenderTargetView(renderSetup->currentRTV(), { 0.0f, 0.0f, 0.0f, 1.0f });

		//CPU_MARKER(apiCmd.api(), "UI Draw command buffer");
		//GPU_MARKER(apiCmd, "UI Draw command buffer");

		//engine::vector<PacketText> textPackets;
		//engine::vector<engine::Rectangle> scissors;
		for (auto&& packet : m_commands)
		{
			switch (packet.type)
			{
				case CommandType::Rectangle:
				{
					if (!ListInUse) TakeListInUse();
					recordRectangle(currentList, rtv, packet);
					break;
				}
				case CommandType::Image:
				{
					if (!ListInUse) TakeListInUse();
					recordImage(currentList, rtv, packet);
					break;
				}
				case CommandType::Text:
				{
					if (!ListInUse) TakeListInUse();
					recordText(currentList, rtv, packet);
					//textPackets.emplace_back(packet.text);
					//scissors.emplace_back(packet.scissor);
					break;
				}
				case CommandType::CommandList:
				{
					if (ListInUse) EndList();
					RenderCommandList renderCommandList;
					renderCommandList.cmdList = std::move(static_cast<PacketCommandList*>(packet.data)->cmdList);
					renderCommandList.renderer = std::move(static_cast<PacketCommandList*>(packet.data)->renderer);
					lists.emplace_back(std::move(renderCommandList));
					break;
				}
			}
		}
		//recordText(cmd, rtv, textPackets, scissors);
		if (!ListInUse) TakeListInUse();
		currentList.transition(renderSetup->currentRTV(), engine::ResourceState::Present);
		if (ListInUse) EndList();

		// this will cause problems. need to fence this.
		m_font->freeTemporaryAllocations();

		return lists;
	}

	void DrawCommandBuffer::pushTransform(const GlobalTransform& transform)
	{
		if (m_transformStack.size())
		{
			auto current = m_transformStack.top();

			engine::Rectangle currentObjectRect{ 
				current.objectX, 
				current.objectY, 
				current.objectX + current.objectWidth, 
				current.objectY + current.objectHeight };

			engine::Rectangle newObjectRect{ 
				currentObjectRect.left + transform.objectX,
				currentObjectRect.top + transform.objectY,
				currentObjectRect.left + transform.objectX + transform.objectWidth,
				currentObjectRect.top + transform.objectY + transform.objectHeight };

			auto objectIntersectRect = newObjectRect;// currentRect.intersect(newRect);

			engine::Rectangle currentClipRect{
				current.clipX,
				current.clipY,
				current.clipX + current.clipWidth,
				current.clipY + current.clipHeight };

			engine::Rectangle newClipRect{
				currentClipRect.left + transform.clipX,
				currentClipRect.top + transform.clipY,
				currentClipRect.left + transform.clipX + transform.clipWidth,
				currentClipRect.top + transform.clipY + transform.clipHeight };

			auto clipIntersectRect = currentClipRect.intersect(newClipRect);

			m_transformStack.push(DrawCommandBuffer::GlobalTransform{ 
				static_cast<int>(objectIntersectRect.left), 
				static_cast<int>(objectIntersectRect.top),
				objectIntersectRect.left < objectIntersectRect.right ? static_cast<int>(objectIntersectRect.width()) : 0,
				objectIntersectRect.top < objectIntersectRect.bottom ? static_cast<int>(objectIntersectRect.height()) : 0,
				static_cast<int>(clipIntersectRect.left),
				static_cast<int>(clipIntersectRect.top),
				clipIntersectRect.left < clipIntersectRect.right ? static_cast<int>(clipIntersectRect.width()) : 0,
				clipIntersectRect.top < clipIntersectRect.bottom ? static_cast<int>(clipIntersectRect.height()) : 0});

			//auto newTransform = current;
			//newTransform.x += transform.x;
			//newTransform.y += transform.y;
			//newTransform.clipWidth = transform.clipWidth;
			//newTransform.clipHeight = transform.clipHeight;
			//if (newTransform.x + newTransform.clipWidth > current.x + current.clipWidth)
			//	newTransform.clipWidth = std::max(0, std::min(newTransform.clipWidth, current.x + current.clipWidth - newTransform.x));
			//if (newTransform.y + newTransform.clipHeight > current.y + current.clipHeight)
			//	newTransform.clipHeight = std::max(0, std::min(newTransform.clipHeight, current.y + current.clipHeight - newTransform.y));
			//m_transformStack.push(newTransform);
		}
		else
			m_transformStack.push(transform);
	}

	DrawCommandBuffer::GlobalTransform DrawCommandBuffer::popTransform()
	{
		auto res = m_transformStack.top();
		m_transformStack.pop();
		return res;
	}

	DrawCommandBuffer::GlobalTransform DrawCommandBuffer::currentTransform() const
	{
		return m_transformStack.top();
	}

	void DrawCommandBuffer::recordRectangle(engine::CommandList& cmd, engine::TextureRTV rtv, const CommandPacket& packet)
	{
		CPU_MARKER(cmd.api(), "UI Rectangle");
		GPU_MARKER(cmd, "UI Rectangle");

		auto p = static_cast<PacketRectangle*>(packet.data);

		cmd.setRenderTargets({ rtv });
		cmd.setScissorRects({ packet.scissor });
		m_drawRectangle.vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
		m_drawRectangle.vs.x = p->x;
		m_drawRectangle.vs.y = p->y;
		m_drawRectangle.vs.width = p->width;
		m_drawRectangle.vs.height = p->height;
		m_drawRectangle.ps.color = { p->r, p->g, p->b, p->a };
		cmd.bindPipe(m_drawRectangle);
		cmd.draw(4);
	}

	void DrawCommandBuffer::recordImage(engine::CommandList& cmd, engine::TextureRTV rtv, const CommandPacket& packet)
	{
		CPU_MARKER(cmd.api(), "UI Image");
		GPU_MARKER(cmd, "UI Image");

		engine::TextureSRV src;

		auto p = static_cast<PacketImage*>(packet.data);

		if (!p->image && !p->texture)
			return;

		if (p->image)
		{
			auto found = m_textures.find(p->image);
			if (found == m_textures.end())
			{
				auto image = p->image;
				m_textures[p->image] = m_device.createTextureSRV(engine::TextureDescription()
					.name("DrawCommandBufferTexture")
					.width(static_cast<uint32_t>(image->width()))
					.height(static_cast<uint32_t>(image->height()))
					.format(srgbFormat(image->format()))
					.arraySlices(static_cast<uint32_t>(image->arraySlices()))
					.mipLevels(image->mipCount() == 0 ? 1 : static_cast<uint32_t>(image->mipCount()))
					.setInitialData(engine::TextureDescription::InitialData(
						tools::ByteRange(
							image->data(),
							image->data() + image->bytes()),
						static_cast<uint32_t>(image->width()), static_cast<uint32_t>(image->width() * image->height()))));
				found = m_textures.find(p->image);
			}
			src = found->second;
		}
		else
		{
			src = p->texture;
		}

		if (src.format() != engine::Format::R8_UINT)
		{
			cmd.setRenderTargets({ rtv });
			cmd.setScissorRects({ packet.scissor });
			m_drawImage.vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
			m_drawImage.vs.x = p->x;
			m_drawImage.vs.y = p->y;
			m_drawImage.vs.width = p->width;
			m_drawImage.vs.height = p->height;
			m_drawImage.ps.image = src;
			cmd.bindPipe(m_drawImage);
			cmd.draw(4);
		}
		else
		{
			cmd.setRenderTargets({ rtv });
			cmd.setScissorRects({ packet.scissor });
			m_drawImageUint.vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
			m_drawImageUint.vs.x = p->x;
			m_drawImageUint.vs.y = p->y;
			m_drawImageUint.vs.width = p->width;
			m_drawImageUint.vs.height = p->height;
			m_drawImageUint.ps.image = src;
			m_drawImageUint.ps.size = { static_cast<float>(p->image->width()), static_cast<float>(p->image->height()) };
			cmd.bindPipe(m_drawImageUint);
			cmd.draw(4);
		}
	}

	void DrawCommandBuffer::resizeGlyphScissorMask(uint32_t width, uint32_t height)
	{
		if (!m_glyphScissorMaskRTV.resource().valid() ||
			width != m_glyphScissorMaskRTV.resource().width() ||
			height != m_glyphScissorMaskRTV.resource().height())
		{
			m_glyphScissorMaskRTV = m_device.createTextureRTV(engine::TextureDescription()
				.width(width)
				.height(height)
				.format(engine::Format::R32_UINT)
				.usage(engine::ResourceUsage::GpuRenderTargetReadWrite)
				.name("Lighting target")
				.dimension(engine::ResourceDimension::Texture2D)
				.optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));
			m_glyphScissorMaskSRV = m_device.createTextureSRV(m_glyphScissorMaskRTV);
		}
	}

	void DrawCommandBuffer::recordText(
		engine::CommandList& cmd, 
		engine::TextureRTV /*rtv*/,
		const engine::vector<PacketText>& /*textPackets*/,
		const engine::vector<engine::Rectangle>& /*scissors*/)
	{
		CPU_MARKER(cmd.api(), "UI Text");
		GPU_MARKER(cmd, "UI Text");
#if 0
		engine::vector<engine::Font::GlyphRenderNodeData> allGlyphs;
		engine::vector<engine::Vector2f> glyphPositions;
		engine::TextureSRV glyphTexture;
		for (auto&& text : textPackets)
		{
			if (!text.text || strlen(text.text) == 0 || text.text[0] == '\n')
				continue;

			auto grenderData = m_font->renderText(text.text);
			// handle changed texture
			glyphTexture = grenderData.texture;
			allGlyphs.reserve(allGlyphs.size() + std::distance(grenderData.nodes.begin(), grenderData.nodes.end()));
			allGlyphs.insert(allGlyphs.end(), grenderData.nodes.begin(), grenderData.nodes.end());
			
			glyphPositions.reserve(glyphPositions.size() + std::distance(grenderData.nodes.begin(), grenderData.nodes.end()));
			for(int i = 0; i < grenderData.nodes.size(); ++i)
				glyphPositions.insert(glyphPositions.end(), engine::Vector2f{ static_cast<float>(text.x), static_cast<float>(text.y) });
		}
		
		auto alloc = m_uploadRing.allocate(allGlyphs.size());
		auto offset = m_uploadRing.offset(alloc.ptr);
		memcpy(
			m_uploadPtr + offset,
			&allGlyphs[0],
			sizeof(engine::Font::GlyphRenderNodeData) * allGlyphs.size());

		auto glyph_alloc = m_glyphPositionsUploadRing.allocate(glyphPositions.size());
		auto glyph_offset = m_glyphPositionsUploadRing.offset(glyph_alloc.ptr);
		memcpy(
			m_glyphPositionsUploadPtr + glyph_offset,
			&glyphPositions[0],
			sizeof(engine::Vector2f) * glyphPositions.size());

		{
			CPU_MARKER(cmd.api(), "UI Text scissor mask");
			GPU_MARKER(cmd, "UI Text scissor mask");
			
			resizeGlyphScissorMask(rtv.width(), rtv.height());

			cmd.clearRenderTargetView(m_glyphScissorMaskRTV);

			engine::vector<RectangleItem> rectangles;
			for (int i = 0; i < scissors.size(); ++i)
			{
				RectangleItem item{
					float2{ static_cast<float>(scissors[i].left),
							static_cast<float>(scissors[i].top) },
					float2{ static_cast<float>(scissors[i].width()),
							static_cast<float>(scissors[i].height()) },
					uint(i+1),
					uint3{}
				};
				rectangles.emplace_back(item);
			}

			auto rectAllocation = m_rectanglesUploadRing.allocate(rectangles.size());
			auto rectOffset = m_rectanglesUploadRing.offset(rectAllocation.ptr);
			memcpy(
				m_rectanglesUploadPtr + rectOffset,
				&rectangles[0],
				sizeof(RectangleItem) * rectangles.size());

			cmd.setRenderTargets({ m_glyphScissorMaskRTV });
			m_drawRectangles.vs.rectangles = m_rectanglesUploadBufferSRV;
			m_drawRectangles.vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
			m_drawRectangles.vs.startIndex.x = rectOffset;
			cmd.bindPipe(m_drawRectangles);
			cmd.draw(6 * rectangles.size());

			m_rectanglesUploadRing.free(rectAllocation);
		}

		// draw glyphs
		{
			CPU_MARKER(cmd.api(), "UI Text Glyph render");
			GPU_MARKER(cmd, "UI Text Glyph render");

			cmd.setRenderTargets({ rtv });
			//cmd.setScissorRects(scissors);
			m_drawText.vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
			m_drawText.vs.x = 0;//packet.image.x;
			m_drawText.vs.y = 0;//packet.image.y;
			m_drawText.vs.width = glyphTexture.width();
			m_drawText.vs.height = glyphTexture.height();
			m_drawText.vs.flipUV = { 0, 0 };
			m_drawText.vs.startIndex.x = offset;
			m_drawText.vs.startIndex.y = glyph_offset;
			m_drawText.vs.glyphs = m_uploadBufferSRV;
			m_drawText.vs.glyphPositions = m_glyphPositionsUploadBufferSRV;

			m_drawText.ps.image = glyphTexture;
			m_drawText.ps.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			cmd.bindPipe(m_drawText);
			cmd.draw(6 * allGlyphs.size());

			m_uploadRing.free(alloc);
			m_glyphPositionsUploadRing.free(glyph_alloc);
		}
#endif
	}

	void DrawCommandBuffer::recordText(engine::CommandList& cmd, engine::TextureRTV rtv, const CommandPacket& packet)
	{
		CPU_MARKER(cmd.api(), "UI Text");
		GPU_MARKER(cmd, "UI Text");

		auto p = static_cast<PacketText*>(packet.data);

		if (!p->text || strlen(p->text) == 0 || p->text[0] == '\n')
			return;

		//engine::vector<engine::Font::GlyphRenderNodeData> allGlyphs;
		engine::vector<engine::Vector2f> glyphPositions;
		auto grenderData = m_font->renderText(p->text);
		// handle changed texture
		//allGlyphs.reserve(allGlyphs.size() + std::distance(grenderData.nodes.begin(), grenderData.nodes.end()));
		//allGlyphs.insert(allGlyphs.end(), grenderData.nodes.begin(), grenderData.nodes.end());

		glyphPositions.reserve(glyphPositions.size() + grenderData.nodeCount);
		for (int i = 0; i < grenderData.nodeCount; ++i)
			glyphPositions.insert(glyphPositions.end(), engine::Vector2f{ static_cast<float>(p->x), static_cast<float>(p->y) });

		auto alloc = m_uploadRing.allocate(grenderData.nodeCount);
		auto offset = m_uploadRing.offset(alloc.ptr);
		memcpy(
			m_uploadPtr + offset,
			&grenderData.nodes[0],
			sizeof(engine::Font::GlyphRenderNodeData) * grenderData.nodeCount);

		cmd.setRenderTargets({ rtv });
		cmd.setScissorRects({ packet.scissor });
		m_drawTextSimple.vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
		m_drawTextSimple.vs.x = 0;
		m_drawTextSimple.vs.y = 0;
		m_drawTextSimple.vs.width = static_cast<float>(grenderData.texture.width());
		m_drawTextSimple.vs.height = static_cast<float>(grenderData.texture.height());
		m_drawTextSimple.vs.flipUV = { 0, 0 };
		m_drawTextSimple.vs.startIndex.x = static_cast<uint32_t>(offset);
		m_drawTextSimple.vs.startIndex.y = 0;
		m_drawTextSimple.vs.glyphs = m_uploadBufferSRV;
		m_drawTextSimple.vs.position = float2{ static_cast<float>(p->x), static_cast<float>(p->y) };

		m_drawTextSimple.ps.image = grenderData.texture;
		m_drawTextSimple.ps.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		cmd.bindPipe(m_drawTextSimple);
		cmd.draw(6 * grenderData.nodeCount);

		m_uploadRing.free(alloc);
	}
}
