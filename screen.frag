#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effectType; // 0=None, 1=Invert, 2=Grayscale, 3=Sharpen, 4=Blur, 5=Edge Detect

const float offset = 1.0 / 300.0;  

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;

    // 0. Normal
    if (effectType == 0) {
        FragColor = vec4(col, 1.0);
    }
    // 1. Inversion (Zombie vision?)
    else if (effectType == 1) {
        FragColor = vec4(1.0 - col, 1.0);
    }
    // 2. Grayscale (Noir)
    else if (effectType == 2) {
        float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
        FragColor = vec4(average, average, average, 1.0);
    }
    // 3. Kernel Effects (Sharpen / Blur / Edge)
    else {
        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right    
        );

        float kernel[9];
        
        // Sharpen
        if (effectType == 3) {
            kernel = float[](
                -1, -1, -1,
                -1,  9, -1,
                -1, -1, -1
            );
        }
        // Blur
        else if (effectType == 4) {
            float v = 1.0 / 16.0;
            kernel = float[](
                1.0*v, 2.0*v, 1.0*v,
                2.0*v, 4.0*v, 2.0*v,
                1.0*v, 2.0*v, 1.0*v
            );
        }
        // Edge Detection
        else if (effectType == 5) {
            kernel = float[](
                1,  1,  1,
                1, -8,  1,
                1,  1,  1
            );
        }

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];
        
        FragColor = vec4(col, 1.0);
    }
}