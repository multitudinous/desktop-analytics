#version 400

uniform mat4 jagModelViewProjMatrix;
uniform mat4 jagModelViewMatrix;
uniform mat3 jagModelViewInvTransMatrix;

uniform LightingLight {
    vec4 position;
    vec4 ambient, diffuse, specular;
} light;
uniform LightingMaterialBack {
    vec4 ambient, diffuse, specular;
    float shininess;
} back;
uniform LightingMaterialFront {
    vec4 ambient, diffuse, specular;
    float shininess;
} front;

in vec4 vertex;
in vec3 normal;
in vec2 texcoord2;

struct Products {
    vec4 ambient, diffuse, specular;
};
out Products backProduct;
out Products frontProduct;


#ifdef USE_TANGENT_SPACE
    in vec4 tangent;
    in vec4 binormal;

    out vec3 tanLightVector;
    out vec3 tanViewVector;
#else
    out vec4 ecVertex;
    out vec3 ecNormal;
#endif

out float dotEye;


void main( void )
{
    gl_Position = jagModelViewProjMatrix * vertex;

    vec4 ecPosition = jagModelViewMatrix * vertex;
    //gl_ClipVertex = ecPosition;

#ifdef USE_TANGENT_SPACE

    //Convert tangent, binormal, and normal into eye coordinates
    mat3 TBNMatrix = mat3( jagModelViewMatrix[0].xyz, jagModelViewMatrix[1].xyz, jagModelViewMatrix[2].xyz ) *
                        mat3( tangent.xyz, binormal.xyz, normal );

    //Convert light vector into tangent space
    tanLightVector = light.position.xyz;
    if( light.position.w > 0.001 )
        tanLightVector -= ecPosition.xyz;
    tanLightVector *= TBNMatrix;

    //Convert view vector into tangent space
    tanViewVector = ecPosition.xyz;
    tanViewVector *= TBNMatrix;

    vec3 ecNormal = normalize( jagModelViewInvTransMatrix * normal );

#else

    ecNormal = normalize( jagModelViewInvTransMatrix * normal );
    ecVertex = ecPosition;

#endif

    backProduct.ambient = back.ambient * light.ambient;
    backProduct.diffuse = back.diffuse * light.diffuse;
    backProduct.specular = back.specular * light.specular;
    frontProduct.ambient = front.ambient * light.ambient;
    frontProduct.diffuse = front.diffuse * light.diffuse;
    frontProduct.specular = front.specular * light.specular;

    // This tells the fragment shader whether we're a front or back face.
    dotEye = dot( ecNormal, -ecPosition.xyz );
}
