#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

// WARNING: Replacing this file may very likely result in game CRASH !!!
// Also attempting to cover this file via texture loader will definitely result in vain.
// If you hope to use your own shader file, apply advanced mode in mod settings and put your shader file in config folder.
// If this file is already broken, get it from my github repo or redownload the whole mod will fix this.

// 0=static 1=gradient 2=pulse 3=huecycle 4=progress
uniform int mode;

// static color
uniform vec3 sc;
uniform float alpha;

// progress
uniform float progress;

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

// hue wheel with a flat brightness
// Perceived brightness weights (Rec.601)
const vec3 HUE_LUMA = vec3(0.299, 0.587, 0.114);

// Brightness all hues are pulled towards, and how strongly they are pulled.
const float HUE_TARGET = 0.55;
const float HUE_EVEN = 0.8;

// fully saturated hue in [0, 1) -> rgb, exact 1/6 segments so the wheel has
// no visible seams where two segments meet
vec3 hue2rgb(float h) {
    h = fract(h) * 6.0;
    if (h < 1.0) return vec3(1.0, h, 0.0);
    if (h < 2.0) return vec3(2.0 - h, 1.0, 0.0);
    if (h < 3.0) return vec3(0.0, 1.0, h - 2.0);
    if (h < 4.0) return vec3(0.0, 4.0 - h, 1.0);
    if (h < 5.0) return vec3(h - 4.0, 0.0, 1.0);
    return vec3(1.0, 0.0, 6.0 - h);
}

// satu = 0 -> one flat gray, satu = 1 -> full hue, brit = overall brightness
vec3 rainbow(float t) {
    vec3 hue = hue2rgb(t);
    float l = dot(hue, HUE_LUMA);

    // pull this hue towards the common brightness
    float want = l + (HUE_TARGET - l) * HUE_EVEN;
    if (l > HUE_TARGET) {
        // too bright (e.g. yellow): darken it, saturation is kept
        hue *= want / max(l, 0.0001);
    } else {
        // too dark (e.g. blue): lift it towards white until it is bright enough
        hue = mix(hue, vec3(1.0), (want - l) / max(1.0 - l, 0.0001));
    }

    // blend against the neutral of the same brightness, so 0% saturation is a
    // single flat gray instead of a light/dark striped gradient
    return clamp(mix(vec3(HUE_TARGET), hue, satu) * brit, 0.0, 1.0);
}

void main() {
    vec4 originalColor = texture2D(CC_Texture0, v_texCoord);
    vec4 color = vec4(1.0);

    if (mode == 0)
        color = vec4(sc, alpha);
    else if (mode == 1)
        color = mix(colorl, colorr, v_texCoord.x);        
    else if (mode == 2)
        color = mix(colorl, colorr, phase); //1.0 - abs(2.0 * phase - 1.0)
    else if (mode == 3)
        color = vec4(rainbow(v_texCoord.x * freq + phase), 1.0);
    else if (mode == 4)
        color = mix(colorl, colorr, progress / 100.0);

    gl_FragColor = originalColor * vec4(color.rgb * color.a, color.a);
}
