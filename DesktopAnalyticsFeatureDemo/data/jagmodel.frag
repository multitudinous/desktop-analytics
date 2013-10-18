#version 400


// 'normal' must be normalized.
vec4 lighting( in vec4 ambProd, in vec4 diffProd, in vec4 specProd, in float specExp,
    in vec3 viewVec, in vec3 normal, in vec3 lightVec )
{
    float diffDot = dot( normal, lightVec );
    vec4 diff = diffProd * max( diffDot, 0. );

    vec4 spec = vec4( 0., 0., 0., 0. );
    if( ( specExp > 0. ) && ( diffDot > 0. ) )
    {
        vec3 reflectVec = reflect( lightVec, normal ); // lightVec and normal are already normalized,
        spec = specProd * pow( max( dot( reflectVec, viewVec ), 0. ), specExp );
    }

    return( ambProd + diff + spec );
}


const bool twoSided = true;
const vec4 lightModelAmbient = vec4( 0., 0., 0., 0. );

#ifdef USE_TANGENT_SPACE
    in vec3 tanLightVector;
    in vec3 tanViewVector;
#else
    in vec4 ecVertex;
    in vec3 ecNormal;
#endif

in float dotEye;

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

uniform vec4 hlc;
uniform bool hlo;

struct Products {
    vec4 ambient, diffuse, specular;
};
in Products backProduct;
in Products frontProduct;



layout (location =0) out vec4 fragData;
layout (location =1) out vec4 highlightData;


void main( void )
{
#ifdef USE_TANGENT_SPACE

    vec3 lightVec = normalize( tanLightVector );
    vec3 viewVec = normalize( tanViewVector );

    // If performing bump mapping, perturb normal by bumpmap here.
    vec3 normal = vec3( 0., 0., 1. );

#else

    vec3 lightVec = light.position.xyz;
    if( light.position.w > 0.001 )
        lightVec = normalize( lightVec - ecVertex.xyz );
    vec3 viewVec = normalize( ecVertex.xyz );
    vec3 normal = normalize( ecNormal );

#endif

    vec4 color;
    if( twoSided && ( dotEye < 0. ) )
    {
        // Backface lighting.
        color = lighting( backProduct.ambient + lightModelAmbient,
            backProduct.diffuse, backProduct.specular,
            back.shininess, viewVec, -normal, lightVec );
    }
    else
    {
        // Frontface / default lighting.
        color = lighting( frontProduct.ambient + lightModelAmbient,
            frontProduct.diffuse, frontProduct.specular,
            front.shininess, viewVec, normal, lightVec );
    }
    if(hlo == true) {
       fragData = color;
	highlightData = hlc;
    }
    else {
    fragData = color;
    }
}
