#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPosition;

uniform int useNormalForColor = 0;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;


// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
    float attenuation;
    float shine;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            float attenuation = 1;

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);

                if (lights[i].attenuation > 0)
                    attenuation = clamp( lights[i].attenuation / distance(fragPosition, lights[i].position), 0.0, 1.0);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL * attenuation;

            float specCo = 0.0;
            if (NdotL > 0.0 && lights[i].shine > 0)
                specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), lights[i].shine);
            specular += specCo;
        }
    }

    finalColor = (texelColor*((colDiffuse + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0)*colDiffuse;

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));

    if(useNormalForColor != 0)
        finalColor = texelColor * colDiffuse * vec4(fragNormal, 1);
}

