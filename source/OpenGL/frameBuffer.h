#pragma once
#include <glew.h>

class frameBuffer
{
	unsigned int m_id;
	unsigned int colorBuffer;
	unsigned int rboDepth;
public:
	frameBuffer() :colorBuffer(0), rboDepth(0)
	{
		glGenFramebuffers(1, &m_id);
	}

	~frameBuffer()
	{
		if(colorBuffer)
		glDeleteTextures(1, &colorBuffer);
		if(rboDepth)
		glDeleteRenderbuffers(1, &rboDepth);
		glDeleteFramebuffers(1, &m_id);
	}

	inline void attachColorBuffer(const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT, const unsigned int internalFormat = GL_RGBA, const unsigned int format = GL_RGBA)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glGenTextures(1, &colorBuffer);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, SCREEN_WIDTH, SCREEN_HEIGHT, 0, format, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	inline void attachDepthBuffer(const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT)
	{
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	}

	inline void blitFbo(const unsigned int srcFbo, const unsigned int destFbo, const unsigned int srcW, const unsigned int srcH,
		const unsigned int destW, const unsigned int destH, const unsigned int blitFlag = GL_COLOR_BUFFER_BIT, const unsigned int textureFilter = GL_NEAREST)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFbo);
		glBlitFramebuffer(0, 0, srcW, srcH, 0, 0, destW, destH, blitFlag, textureFilter);
	}

	inline unsigned int getId() const
	{
		return m_id;
	}

	inline unsigned int getColor() const
	{
		return colorBuffer;
	}

	inline void bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	}

	inline void unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};