#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

// WARNING: Replacing this file may very likely result in game CRASH !!!
// Also attempting to cover this file via texture loader will definitely result in vain.
// If you hope to use your own shader file, apply advanced mode in mod settings and put your shader file in config folder.
// If this file is already broken, get it from my github repo or redownload the whole mod will fix this.

// 0=static 1=gradient 2=pulse 3=huecycle
uniform int mode;

// static color
uniform vec4 sc;

// pulse & huecycle
uniform float phase;

// gradient & pulse
uniform vec4 colorl;
uniform vec4 colorr;

// hue cycle
uniform float freq;
uniform float satu;
uniform float brit;

// texture
uniform sampler2D CC_Texture0;

varying vec2 v_texCoord;
varying vec4 v_color;

// gen rainbow
vec3 rainbow(float t) {
    // normalize
    t = fract(t);
    
    vec3 color = vec3(1.0);
    
    if(t < 1.0/6.0) {
        color = mix(vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 0.0), t * 6.0);
    } else if(t < 1.0/3.0) {
        color = mix(vec3(1.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0), (t - 0.166) * 6.0);
    } else if(t < 0.5) {
        color = mix(vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 1.0), (t - 0.333) * 6.0);
    } else if(t < 2.0/3.0) {
        color = mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 0.0, 1.0), (t - 0.5) * 6.0);
    } else if(t < 5.0/6.0) {
        color = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), (t - 0.666) * 6.0);
    } else {
        color = mix(vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), (t - 0.833) * 6.0);
    }
    
    // apply luminance and brightness
    return mix(vec3(dot(color, vec3(0.299, 0.587, 0.114))), color, satu) * brit;
}


void main() {
    vec4 originalColor = texture2D(CC_Texture0, v_texCoord);
    vec4 color = vec4(1.0);

    if (mode == 0)
        color = sc;     
    else if (mode == 1)
        color = mix(colorl, colorr, v_texCoord.x);        
    else if (mode == 2)
        color = mix(colorl, colorr, phase); //1.0 - abs(2.0 * phase - 1.0)
    else if (mode == 3)
        color = vec4(rainbow(v_texCoord.x * freq + phase), 1.0);

    gl_FragColor = originalColor * color;
}