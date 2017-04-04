struct DirectionalLight
{
	float3 direction;
	float pad;

	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;
};

struct PointLight
{
	float3 position;
	float range;

	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;

	float3 attenu;
	float pad;
};

struct SpotLight
{
	float3 position;
	float range;

	float3 direction;
	float spot;

	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;

	float3 attenu;
	float pad;
};

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;// w = SpecPower
};


void ComputeDirectionalLight( DirectionalLight light , Material mat , float4 color , float3 normal , float3 view ,
	out float4 diffuse , out float4 specular , out float4 ambient )
{
	diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

	float3 lightDir = -light.direction;
	float diffuseFactor = dot( lightDir , normal );

	diffuse = light.diffuseColor * mat.diffuse * max( diffuseFactor , 0 ) * color;
	ambient = light.ambientColor * mat.ambient;
	
	float hasSpec = smoothstep( -0.1f , 0.1f , diffuseFactor );
	float specFactor = pow( max( dot( reflect( -lightDir , normal ) , view ) , 0 ) , mat.specular.w );
	specular = light.specularColor * mat.specular * specFactor * hasSpec;
}

void ComputePointLight( PointLight light , Material mat , float3 pos , float4 color , float3 normal , float3 view ,
	out float4 diffuse , out float4 specular , out float4 ambient )
{
	diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

	float3 lightDir = light.position - pos;
	float lightLen = length( lightDir );
	if ( lightLen > light.range ) return;

	lightDir = lightDir / lightLen;
	float diffuseFactor = dot( lightDir , normal );

	diffuse = light.diffuseColor * mat.diffuse * max( diffuseFactor , 0 ) * color;
	ambient = light.ambientColor * mat.ambient;

	float hasSpec = smoothstep( -0.1f , 0.1f , diffuseFactor );
	float specFactor = pow( max( dot( reflect( -lightDir , normal ) , view ) , 0 ) , mat.specular.w );
	specular = light.specularColor * mat.specular * specFactor * hasSpec;

	float attenu = 1 / ( dot( light.attenu , float3( 1 , lightLen , lightLen *lightLen ) ) );
	diffuse *= attenu;
	specular *= attenu;
}

void ComputeSpotLight( SpotLight light , Material mat , float3 pos , float4 color , float3 normal , float3 view ,
	out float4 diffuse , out float4 specular , out float4 ambient )
{
	diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

	float3 lightDir = light.position - pos;
	float lightLen = length( lightDir );
	if ( lightLen > light.range ) return;

	lightDir = lightDir / lightLen;
	float diffuseFactor = dot( lightDir , normal );

	diffuse = light.diffuseColor * mat.diffuse * max( diffuseFactor , 0 ) * color;
	ambient = light.ambientColor * mat.ambient;

	float hasSpec = smoothstep( -0.1f , 0.1f , diffuseFactor );
	float specFactor = pow( max( dot( reflect( -lightDir , normal ) , view ) , 0 ) , mat.specular.w );
	specular = light.specularColor * mat.specular * specFactor * hasSpec;

	float spotFactor = pow( max( dot( -lightDir , light.direction ) , 0 ) , light.spot );
	float attenu = spotFactor / ( dot( light.attenu , float3( 1 , lightLen , lightLen *lightLen ) ) );
	diffuse *= attenu;
	specular *= attenu;
	ambient *= spotFactor;
}