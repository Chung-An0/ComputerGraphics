#include "Texture.h"

#include <string>
#include <vector>

// stb_image 구현 (한 번만 정의)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint Texture::Load(const char* filepath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 텍스처 파라미터 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 이미지 로드를 위한 변수 초기화
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string path = filepath;
    // Helper lambda to attempt loading a texture from a given path
    auto tryLoad = [&](const std::string& p) -> bool {
        data = stbi_load(p.c_str(), &width, &height, &channels, 0);
        if (data) {
            // Copy back the path so we can log which file loaded
            path = p;
            return true;
        }
        return false;
        };
    // First attempt: original path
    bool loaded = tryLoad(path);
    // If failed, try switching between .jpg and .png extensions
    if (!loaded) {
        // Determine file extension
        size_t dotPos = path.find_last_of('.');
        if (dotPos != std::string::npos) {
            std::string base = path.substr(0, dotPos);
            std::string ext = path.substr(dotPos);
            if (ext == ".jpg" || ext == ".jpeg") {
                loaded = tryLoad(base + ".png");
            }
            else if (ext == ".png") {
                loaded = tryLoad(base + ".jpg");
            }
        }
    }
    // If still failed, try prefixing with one and two parent directories
    if (!loaded) {
        std::vector<std::string> prefixes = { "../", "../../", "../../../" };
        for (const auto& prefix : prefixes) {
            if (loaded) break;
            std::string basePath = prefix + path;
            loaded = tryLoad(basePath);
            if (!loaded) {
                // Try alternative extension with the same prefix
                size_t dotPos = path.find_last_of('.');
                if (dotPos != std::string::npos) {
                    std::string base = path.substr(0, dotPos);
                    std::string ext = path.substr(dotPos);
                    if (ext == ".jpg" || ext == ".jpeg") {
                        loaded = tryLoad(prefix + base + ".png");
                    }
                    else if (ext == ".png") {
                        loaded = tryLoad(prefix + base + ".jpg");
                    }
                }
            }
        }
    }
    if (loaded && data) {
        GLenum format = GL_RGB;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        cout << "Texture loaded: " << path << " (" << width << "x" << height << ")" << endl;
    }
    else {
        cout << "Failed to load texture: " << filepath << endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

GLuint Texture::LoadCubemap(vector<string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
        if (data) {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }
        else {
            cout << "Cubemap failed to load: " << faces[i] << endl;
            stbi_image_free(data);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            return 0;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return textureID;
}


void Texture::Bind(GLuint textureID, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::BindCubemap(GLuint cubemapID, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
}

void Texture::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::UnbindCubemap() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
