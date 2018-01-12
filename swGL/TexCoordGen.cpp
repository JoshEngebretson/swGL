#include "Defines.h"
#include "Log.h"
#include "Vertex.h"
#include "TexCoordGen.h"

namespace SWGL {

    TexCoordGen::TexCoordGen(Vertex &vertexState)

        : m_vertexState(vertexState),
          m_enableMask(0U),
          m_activeTexture(0U) {

    }



    void TexCoordGen::setEnable(unsigned int texCoordIdx, bool isEnabled) {

        if (isEnabled) {

            m_enableMask |= 1U << m_activeTexture;
            m_state[m_activeTexture].enableMask |= 1U << texCoordIdx;
        }
        else {

            m_enableMask &= ~(1U << m_activeTexture);
            m_state[m_activeTexture].enableMask &= ~(1U << texCoordIdx);
        }
    }

    void TexCoordGen::setMode(unsigned int texCoordIdx, GLenum mode) {

        m_state[m_activeTexture].data[texCoordIdx].mode = mode;
    }

    void TexCoordGen::setObjectPlane(unsigned int texCoordIdx, Vector plane) {

        m_state[m_activeTexture].data[texCoordIdx].objectPlane = plane;
    }

    void TexCoordGen::setEyePlane(unsigned int texCoordIdx, Vector plane) {

        m_state[m_activeTexture].data[texCoordIdx].eyePlane = plane;
    }

    void TexCoordGen::setActiveTexture(unsigned int activeTexture) {

        m_activeTexture = activeTexture;
    }



    float TexCoordGen::generateTexCoord(unsigned int texCoordIdx, TexCoordGenState &state) {

        const auto &data = state.data[texCoordIdx];

        switch (data.mode) {

        case GL_EYE_LINEAR:
            LOG("Unimplemented GL_EYE_LINEAR");
            return 0.0f;

        case GL_OBJECT_LINEAR:
            return Vector::dot(m_vertexState.obj, data.objectPlane);

        case GL_SPHERE_MAP:
            LOG("Unimplemented GL_SPHERE_MAP");
            return 0.0f;

        case GL_NORMAL_MAP:
            LOG("Unimplemented GL_NORMAL_MAP");
            return 0.0f;

        case GL_REFLECTION_MAP:
            LOG("Unimplemented GL_REFLECTION_MAP");
            return 0.0f;
        }

        LOG("Invalid mode");
        return 0.0f;
    }



    bool TexCoordGen::isEnabled() {

        return m_enableMask != 0U;
    }

    void TexCoordGen::generate() {

        for (auto i = 0U; i < SWGL_MAX_TEXTURE_UNITS; i++) {

            auto &state = m_state[i];
            if (state.enableMask != 0U) {

                auto &texCoord = m_vertexState.texCoord[i];

                if ((state.enableMask & 1U) != 0U) { texCoord.x() = generateTexCoord(0U, state); }
                if ((state.enableMask & 2U) != 0U) { texCoord.y() = generateTexCoord(1U, state); }
                if ((state.enableMask & 4U) != 0U) { texCoord.z() = generateTexCoord(2U, state); }
                if ((state.enableMask & 8U) != 0U) { texCoord.w() = generateTexCoord(3U, state); }
            }
        }
    }
}