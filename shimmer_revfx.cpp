#include "userrevfx.h"
#include "fixed_math.h"

namespace {

constexpr uint32_t kFdnSize = 16384;
constexpr uint32_t kFdnMask = kFdnSize - 1;

constexpr uint32_t kPitchSize = 4096;
constexpr uint32_t kPitchMask = kPitchSize - 1;
constexpr float kPitchWindow = 1536.0f;
constexpr float kPitchMinDelay = 64.0f;
constexpr float kOctaveUpPhaseStep = 1.0f / kPitchWindow;

constexpr uint32_t kAp0Size = 256;
constexpr uint32_t kAp0Mask = kAp0Size - 1;
constexpr uint32_t kAp1Size = 512;
constexpr uint32_t kAp1Mask = kAp1Size - 1;

float s_fdn0[kFdnSize] __sdram;
float s_fdn1[kFdnSize] __sdram;
float s_fdn2[kFdnSize] __sdram;
float s_fdn3[kFdnSize] __sdram;
float s_pitch[kPitchSize] __sdram;
float s_ap0[kAp0Size] __sdram;
float s_ap1[kAp1Size] __sdram;

uint32_t s_fdn_idx;
uint32_t s_pitch_idx;
uint32_t s_ap0_idx;
uint32_t s_ap1_idx;

float s_pitch_phase;
float s_decay;
float s_wet;
float s_shimmer;
float s_damp;
float s_tone_lp;
float s_hp_lp;
float s_damp0;
float s_damp1;
float s_damp2;
float s_damp3;

inline float clamp01(float x) {
  return (x < 0.0f) ? 0.0f : ((x > 1.0f) ? 1.0f : x);
}

inline float absf_fast(float x) {
  return (x < 0.0f) ? -x : x;
}

inline float lerpf(float a, float b, float t) {
  return a + (b - a) * t;
}

inline float softclip(float x) {
  if (x > 1.35f) {
    return 1.0f;
  }
  if (x < -1.35f) {
    return -1.0f;
  }

  const float y = x * 0.74f;
  return y * (1.5f - 0.5f * y * y);
}

inline float param_to_float(int32_t value) {
  return clamp01(q31_to_f32(value));
}

template <uint32_t N>
void clear_buffer(float (&buffer)[N]) {
  for (uint32_t i = 0; i < N; ++i) {
    buffer[i] = 0.0f;
  }
}

void clear_state() {
  clear_buffer(s_fdn0);
  clear_buffer(s_fdn1);
  clear_buffer(s_fdn2);
  clear_buffer(s_fdn3);
  clear_buffer(s_pitch);
  clear_buffer(s_ap0);
  clear_buffer(s_ap1);

  s_fdn_idx = 0;
  s_pitch_idx = 0;
  s_ap0_idx = 0;
  s_ap1_idx = 0;
  s_pitch_phase = 0.0f;
  s_tone_lp = 0.0f;
  s_hp_lp = 0.0f;
  s_damp0 = 0.0f;
  s_damp1 = 0.0f;
  s_damp2 = 0.0f;
  s_damp3 = 0.0f;
}

inline float allpass0(float x) {
  constexpr float kFeedback = 0.62f;
  const float z = s_ap0[s_ap0_idx];
  const float y = z - x * kFeedback;
  s_ap0[s_ap0_idx] = x + y * kFeedback;
  s_ap0_idx = (s_ap0_idx + 1) & kAp0Mask;
  return y;
}

inline float allpass1(float x) {
  constexpr float kFeedback = 0.58f;
  const float z = s_ap1[s_ap1_idx];
  const float y = z - x * kFeedback;
  s_ap1[s_ap1_idx] = x + y * kFeedback;
  s_ap1_idx = (s_ap1_idx + 1) & kAp1Mask;
  return y;
}

inline float fdn_read(const float *buffer, uint32_t delay) {
  return buffer[(s_fdn_idx - delay) & kFdnMask];
}

inline float pitch_read(float delay) {
  float position = static_cast<float>(s_pitch_idx) - delay;
  if (position < 0.0f) {
    position += static_cast<float>(kPitchSize);
  }

  const uint32_t i0 = static_cast<uint32_t>(position) & kPitchMask;
  const uint32_t i1 = (i0 + 1) & kPitchMask;
  const float frac = position - static_cast<float>(static_cast<uint32_t>(position));
  return lerpf(s_pitch[i0], s_pitch[i1], frac);
}

inline float tri_window(float phase) {
  return 1.0f - absf_fast(phase * 2.0f - 1.0f);
}

float pitch_octave_up(float x) {
  s_pitch[s_pitch_idx] = x;

  float phase_b = s_pitch_phase + 0.5f;
  if (phase_b >= 1.0f) {
    phase_b -= 1.0f;
  }

  const float delay_a = kPitchMinDelay + kPitchWindow * (1.0f - s_pitch_phase);
  const float delay_b = kPitchMinDelay + kPitchWindow * (1.0f - phase_b);
  const float wa = tri_window(s_pitch_phase);
  const float wb = tri_window(phase_b);
  const float y = pitch_read(delay_a) * wa + pitch_read(delay_b) * wb;

  s_pitch_phase += kOctaveUpPhaseStep;
  if (s_pitch_phase >= 1.0f) {
    s_pitch_phase -= 1.0f;
  }

  s_pitch_idx = (s_pitch_idx + 1) & kPitchMask;
  return y;
}

inline void set_time(float x) {
  const float shaped = x * x;
  s_decay = 0.54f + shaped * 0.405f;
  s_damp = 0.28f + x * 0.48f;
}

inline void set_wet(float x) {
  s_wet = x;
}

inline void set_shimmer(float x) {
  s_shimmer = x * x * 0.58f;
}

void set_defaults() {
  set_time(0.65f);
  set_wet(0.38f);
  set_shimmer(0.45f);
}

} // namespace

void REVFX_INIT(uint32_t platform, uint32_t api) {
  (void)platform;
  (void)api;
  clear_state();
  set_defaults();
}

void REVFX_PROCESS(float *xn, uint32_t frames) {
  for (uint32_t i = 0; i < frames; ++i) {
    const float dry_l = xn[i * 2];
    const float dry_r = xn[i * 2 + 1];
    const float input = (dry_l + dry_r) * 0.5f;

    const float r0 = fdn_read(s_fdn0, 4313);
    const float r1 = fdn_read(s_fdn1, 5683);
    const float r2 = fdn_read(s_fdn2, 7211);
    const float r3 = fdn_read(s_fdn3, 9221);

    s_damp0 = lerpf(r0, s_damp0, s_damp);
    s_damp1 = lerpf(r1, s_damp1, s_damp);
    s_damp2 = lerpf(r2, s_damp2, s_damp);
    s_damp3 = lerpf(r3, s_damp3, s_damp);

    const float tail_l = (s_damp0 - s_damp1 + s_damp2 - s_damp3) * 0.45f;
    const float tail_r = (s_damp0 + s_damp1 - s_damp2 - s_damp3) * 0.45f;
    const float tail_m = (tail_l + tail_r) * 0.5f;

    s_hp_lp += 0.018f * (tail_m - s_hp_lp);
    const float bright_tail = tail_m - s_hp_lp;
    const float shimmer = pitch_octave_up(bright_tail) * s_shimmer;

    s_tone_lp += 0.11f * (input - s_tone_lp);
    const float send = allpass1(allpass0(input * 0.30f + s_tone_lp * 0.10f + shimmer));

    const float a = s_damp0;
    const float b = s_damp1;
    const float c = s_damp2;
    const float d = s_damp3;

    s_fdn0[s_fdn_idx] = softclip(send + (a + b + c + d) * (s_decay * 0.5f));
    s_fdn1[s_fdn_idx] = softclip(send + (a - b + c - d) * (s_decay * 0.5f));
    s_fdn2[s_fdn_idx] = softclip(send + (a + b - c - d) * (s_decay * 0.5f));
    s_fdn3[s_fdn_idx] = softclip(send + (a - b - c + d) * (s_decay * 0.5f));
    s_fdn_idx = (s_fdn_idx + 1) & kFdnMask;

    const float wet_l = softclip(tail_l * 1.15f + shimmer * 0.25f);
    const float wet_r = softclip(tail_r * 1.15f + shimmer * 0.20f);
    const float dry_gain = 1.0f - s_wet;
    const float wet_gain = s_wet * 1.18f;

    xn[i * 2] = softclip(dry_l * dry_gain + wet_l * wet_gain);
    xn[i * 2 + 1] = softclip(dry_r * dry_gain + wet_r * wet_gain);
  }
}

void REVFX_PARAM(uint8_t index, int32_t value) {
  const float x = param_to_float(value);

  switch (index) {
  case k_user_revfx_param_time:
    set_time(x);
    break;

  case k_user_revfx_param_depth:
    set_wet(x);
    break;

  case k_user_revfx_param_shift_depth:
    set_shimmer(x);
    break;

  default:
    break;
  }
}

void REVFX_RESUME(void) {
  clear_state();
}

void REVFX_SUSPEND(void) {
}
