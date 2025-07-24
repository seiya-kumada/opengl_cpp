#version 330 core
out vec4 FragColor;

in vec3 vertexColor;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// ライティングパラメータ
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main()
{
    // 環境光
    vec3 ambient = ambientStrength * lightColor;
    
    // 拡散光
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 鏡面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * vertexColor;
    FragColor = vec4(result, 1.0);
} 