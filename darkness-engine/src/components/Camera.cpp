#include "components/Camera.h"
#include "engine/primitives/Math.h"

namespace engine
{
    engine::string projectionToString(const Projection& projection)
    {
        switch (projection)
        {
        case Projection::Orthographic: return "Orthographic";
        case Projection::Perspective: return "Perspective";
        }
        return "Perspective";
    }

    Projection stringToProjection(const engine::string& projection)
    {
        if (projection == "Orthographic")
            return Projection::Orthographic;
        else if (projection == "Perspective")
            return Projection::Perspective;
        return Projection::Orthographic;
    }

    int Camera::width() const
    {
        return m_width.value<int>();
    }

    void Camera::width(int _width)
    {
        m_width.value<int>(_width);
    }

    int Camera::height() const
    {
        return m_height.value<int>();
    }
    void Camera::height(int _height)
    {
        m_height.value<int>(_height);
    }

    float Camera::nearPlane() const
    {
        return m_nearPlane.value<float>();
    }

    void Camera::nearPlane(float _near)
    {
        m_nearPlane.value<float>(_near);
    }

    float Camera::farPlane() const
    {
        return m_farPlane.value<float>();
    }

    void Camera::farPlane(float _far)
    {
        m_farPlane.value<float>(_far);
    }

    float Camera::fieldOfView() const
    {
        return m_fieldOfView.value<float>();
    }

    void Camera::fieldOfView(float _fov)
    {
        m_fieldOfView.value<float>(_fov);
    }

    Projection Camera::projection() const
    {
        return m_projection.value<Projection>();
    }

    void Camera::projection(Projection _projection)
    {
        m_projection.value<Projection>(_projection);
    }

    Quaternionf Camera::rotation() const
    {
        return m_transform->rotation();
    }

    void Camera::rotation(Quaternionf _rotation)
    {
        m_transform->rotation(_rotation);
    }

    Vector3f Camera::position() const
    {
        return m_transform->position();
    }

    void Camera::updateDelta(float delta)
    {
        Vector3f target = m_transform->position() - m_smoothPosition;
        float distance = target.magnitude();
        const float maxSpeed = 10.0f;
        float fspeed = m_followSpeed.value<float>();
        if (fspeed < 0.01f)
        {
            fspeed = 0.01f;
        }
        Vector3f move = target.normalize() * (distance / fspeed) * delta;
        if (move.magnitude() > maxSpeed)
        {
            move = move.normalize() * maxSpeed;
        }
        m_smoothPosition += move;
    }

    Vector3f Camera::smoothPosition() const
    {
        return m_smoothPosition;
    }

    void Camera::position(Vector3f _position)
    {
        m_transform->position(_position);
    }

    Vector3f Camera::target() const
    {
        return m_target;
    }

    void Camera::target(Vector3f target)
    {
        m_target = target;
    };

    Matrix4f Camera::lookAt(
        const Vector3f& position,
        const Vector3f& target,
        const Vector3f& up)
    {
        Vector3f u = up;
        Vector3f zaxis;
        auto toVect = Vector3f(position - target);
        if (toVect != Vector3f{ 0.0f, 0.0f, 0.0f })
            zaxis = toVect.normalize();
        else
            zaxis = Vector3f{ 0.0f, 0.0f, 1.0f };

        Vector3f xaxis = u.cross(zaxis).normalize();
        Vector3f yaxis = zaxis.cross(xaxis);
        
        Matrix4f res = Matrix4f::identity();
        res.m00 = xaxis.x; res.m01 = yaxis.x; res.m02 = zaxis.x; res.m03 = 0;
        res.m10 = xaxis.y; res.m11 = yaxis.y; res.m12 = zaxis.y; res.m13 = 0;
        res.m20 = xaxis.z; res.m21 = yaxis.z; res.m22 = zaxis.z; res.m23 = 0;
        res.m30 = -xaxis.dot(position); res.m31 = -yaxis.dot(position); res.m32 = -zaxis.dot(position); res.m33 = 1;
        return res;

        /*Vector3f f = (target - position).normalize();
        Vector3f s = f.cross(up).normalize();
        Vector3f u = s.cross(f);

        Matrix4f mat = Matrix4f::identity();
        mat.m00 = s.x;
        mat.m10 = s.y;
        mat.m20 = s.z;
        mat.m01 = u.x;
        mat.m11 = u.y;
        mat.m21 = u.z;
        mat.m02 = -f.x;
        mat.m12 = -f.y;
        mat.m22 = -f.z;
        mat.m30 = -s.dot(position);
        mat.m31 = -u.dot(position);
        mat.m32 = f.dot(position);
        return mat;*/
    }

    Matrix4f Camera::viewMatrix() const
    {
        Matrix4f position = Matrix4f::translate(
            m_transform->position().x,
            m_transform->position().y,
            m_transform->position().z);
        Matrix4f scale = Matrix4f::scale(
            m_transform->scale().x,
            m_transform->scale().y,
            m_transform->scale().z);

        Matrix4f modelMatrix = position * m_transform->rotation().toMatrix() * scale;
#ifdef VALIDATE_VIEW_MATRIX
        auto res = modelMatrix.inverse();
        ASSERT(res.toMatrix3().orthonormal(), "Invalid view matrix");
        return res;
#else
        return modelMatrix.inverse();
#endif
    }

    Matrix4f Camera::viewMatrix(Vector3f withPosition) const
    {
        Matrix4f position = Matrix4f::translate(
            withPosition.x,
            withPosition.y,
            withPosition.z);
        Matrix4f scale = Matrix4f::scale(
            m_transform->scale().x,
            m_transform->scale().y,
            m_transform->scale().z);

        Matrix4f modelMatrix = position * m_transform->rotation().toMatrix() * scale;

#ifdef VALIDATE_VIEW_MATRIX
        auto res = modelMatrix.inverse();
        ASSERT(res.toMatrix3().orthonormal(), "Invalid view matrix");
        return res;
#else
        return modelMatrix.inverse();
#endif
    }

    Matrix4f Camera::projectionMatrix() const
    {
        if (m_projection.value<Projection>() == Projection::Perspective)
        {
            return perspectiveMatrix();
        }
        else if (m_projection.value<Projection>() == Projection::Orthographic)
        {
            return orthographicMatrix();
        }
        return perspectiveMatrix();
    }

    Matrix4f Camera::projectionMatrix(Vector2<int> screenSize) const
    {
        if (m_projection.value<Projection>() == Projection::Perspective)
        {
            return perspectiveMatrix(screenSize);
        }
        else if (m_projection.value<Projection>() == Projection::Orthographic)
        {
            return orthographicMatrix(screenSize);
        }
        return perspectiveMatrix(screenSize);
    }

    double halton(int index, int base)
    {
        double res = 0.0f;
        double fraction = 1.0f / (double)base;

        while (index > 0)
        {
            res += (double)(index % base) * fraction;
            index /= base;
            fraction /= (double)base;
        }
        return res;
    }

    void Camera::createHaltonValues()
    {
        m_haltonValues.clear();
        for (int i = 0; i < HaltonValueCount; ++i)
        {
            m_haltonValues.emplace_back(Vector2<double>(halton(i, 2), halton(i, 3)));
        }
    }

    Matrix4f Camera::jitterMatrix(uint64_t frameNumber, Vector2<int> screenSize) const
    {
        if (m_jitteringEnabled)
        {
            auto currentJitter = (m_haltonValues[frameNumber % m_haltonValues.size()] * 2.0) - 1.0;
            return {
                1.0f, 0.0f, 0.0f, static_cast<float>(currentJitter.x) * (1.0f / static_cast<float>(screenSize.x)),
                0.0f, 1.0f, 0.0f, static_cast<float>(currentJitter.y) * (1.0f / static_cast<float>(screenSize.y)),
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
        }
        else
        {
            return {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
        }
    }

    Vector2f Camera::jitterValue(uint64_t frameNumber) const
    {
        if (m_jitteringEnabled)
        {
            auto currentJitter = (m_haltonValues[frameNumber % m_haltonValues.size()] * 2.0) - 1.0;
            return { static_cast<float>(currentJitter.x), static_cast<float>(currentJitter.y) };
        }
        else
            return { 0.0f, 0.0f };
    }

    void Camera::forward(Vector3f forward)
    {
        m_forward = forward;
    }

    void Camera::up(Vector3f up)
    {
        m_up = up;
    }

    Vector3f Camera::up() const
    {
        return m_up;
    }

    void Camera::right(Vector3f right)
    {
        m_right = right;
    }

    Vector3f Camera::right() const
    {
        return m_right;
    }

    Vector3f Camera::forward() const
    {
        return m_forward;
    }

    ViewCornerRays Camera::viewRays() const
    {
        auto forward = m_forward;
        forward.normalize();
        forward *= -1.0f;

        auto nearDistance = nearPlane();
        auto farDistance = farPlane();
        auto fov = fieldOfView();
        auto fovRad = fov * DEG_TO_RAD;
        auto viewRatio = static_cast<float>(width()) / static_cast<float>(height());

        auto nearCenter = forward * nearDistance;
        auto farCenter = forward * farDistance;

        auto nearHeight = 2 * tan(fovRad / 2.0f) * nearDistance;
        auto farHeight = 2 * tan(fovRad / 2.0f) * farDistance;
        auto nearWidth = nearHeight * viewRatio;
        auto farWidth = farHeight * viewRatio;

        auto farTopLeft = farCenter + m_up * (farHeight * 0.5f) - m_right * (farWidth * 0.5f);
        auto farTopRight = farCenter + m_up * (farHeight * 0.5f) + m_right * (farWidth * 0.5f);
        auto farBottomLeft = farCenter - m_up * (farHeight * 0.5f) - m_right * (farWidth * 0.5f);
        auto farBottomRight = farCenter - m_up * (farHeight * 0.5f) + m_right * (farWidth * 0.5f);

        auto nearTopLeft = nearCenter + m_up * (nearHeight * 0.5f) - m_right * (nearWidth * 0.5f);
        auto nearTopRight = nearCenter + m_up * (nearHeight * 0.5f) + m_right * (nearWidth * 0.5f);
        auto nearBottomLeft = nearCenter - m_up * (nearHeight * 0.5f) - m_right * (nearWidth * 0.5f);
        auto nearBottomRight = nearCenter - m_up * (nearHeight * 0.5f) + m_right * (nearWidth * 0.5f);

        //LOG("forward: %f, %f, %f", forward.x, forward.y, forward.z);

        return {
            (farTopLeft - nearTopLeft).normalize(),
            (farTopRight - nearTopRight).normalize(),
            (farBottomLeft - nearBottomLeft).normalize(),
            (farBottomRight - nearBottomRight).normalize()
        };
    }

	ViewCornerRays Camera::nearPlaneCorners() const
	{
		auto forward = m_forward;
		forward.normalize();
		forward *= -1.0f;

		auto nearDistance = nearPlane();
		auto fov = fieldOfView();
		auto fovRad = fov * DEG_TO_RAD;
		auto viewRatio = static_cast<float>(width()) / static_cast<float>(height());

		auto nearCenter = forward * nearDistance;

		auto nearHeight = 2 * tan(fovRad / 2.0f) * nearDistance;
		auto nearWidth = nearHeight * viewRatio;

		auto nearTopLeft = nearCenter + m_up * (nearHeight * 0.5f) - m_right * (nearWidth * 0.5f);
		auto nearTopRight = nearCenter + m_up * (nearHeight * 0.5f) + m_right * (nearWidth * 0.5f);
		auto nearBottomLeft = nearCenter - m_up * (nearHeight * 0.5f) - m_right * (nearWidth * 0.5f);
		auto nearBottomRight = nearCenter - m_up * (nearHeight * 0.5f) + m_right * (nearWidth * 0.5f);

		//LOG("forward: %f, %f, %f", forward.x, forward.y, forward.z);
		float3 pos = position();

		return {
			pos + nearTopLeft,
			pos + nearTopRight,
			pos + nearBottomLeft,
			pos + nearBottomRight
		};
	}

    Matrix4f Camera::orthographicMatrix() const
    {
        // right handed matrix
        float w = static_cast<float>(width());
        float h = static_cast<float>(height());
        float zf = m_nearPlane.value<float>();
        float zn = m_farPlane.value<float>();

        Matrix4f orthographicMat;
        orthographicMat.m00 = 2.0f / w;
        orthographicMat.m11 = 2.0f / h;
        //orthographicMat.m22 = 1.0f / (zn - zf);
        //orthographicMat.m23 = zn / (zn - zf);

        orthographicMat.m22 = 1.0f / (zf - zn);
        orthographicMat.m23 = -zn / (zf - zn);

        orthographicMat.m33 = 1.0f;
        return orthographicMat;
    }

    Matrix4f Camera::perspectiveMatrix() const
    {
        // right handed matrix
        float w = static_cast<float>(width());
        float h = static_cast<float>(height());
        float zn = m_farPlane.value<float>();
        float zf = m_nearPlane.value<float>();
        float fov = m_fieldOfView.value<float>();

        float yScale = 1.0f / std::tanf(DEG_TO_RAD * fov * 0.5f);
        float xScale = yScale / (w / h);

        Matrix4f perspectiveMat;
        perspectiveMat.m00 = xScale;
        perspectiveMat.m11 = yScale;
        perspectiveMat.m22 = zf / (zn - zf);
        perspectiveMat.m23 = zn * zf / (zn - zf);
        perspectiveMat.m32 = -1.0f;
        return perspectiveMat;
    }

    Matrix4f Camera::orthographicMatrix(Vector2<int> screenSize) const
    {
        // right handed matrix
        float w = static_cast<float>(screenSize.x);
        float h = static_cast<float>(screenSize.y);
        float zf = m_nearPlane.value<float>();
        float zn = m_farPlane.value<float>();

        Matrix4f orthographicMat;
        orthographicMat.m00 = 2.0f / w;
        orthographicMat.m11 = 2.0f / h;
        orthographicMat.m22 = 1.0f / (zn - zf);
        orthographicMat.m23 = zn / (zn - zf);
        orthographicMat.m33 = 1.0f;
        return orthographicMat;
    }

    Matrix4f Camera::perspectiveMatrix(Vector2<int> screenSize) const
    {
        // right handed matrix
        float w = static_cast<float>(screenSize.x);
        float h = static_cast<float>(screenSize.y);
        float zn = m_farPlane.value<float>();
        float zf = m_nearPlane.value<float>();
        float fov = m_fieldOfView.value<float>();

        float yScale = 1.0f / std::tanf(DEG_TO_RAD * fov * 0.5f);
        float xScale = yScale / (w / h);

        Matrix4f perspectiveMat;
        perspectiveMat.m00 = xScale;
        perspectiveMat.m11 = yScale;
        perspectiveMat.m22 = zf / (zn - zf);
        perspectiveMat.m23 = zn * zf / (zn - zf);
        perspectiveMat.m32 = -1.0f;
        return perspectiveMat;
    }
}
