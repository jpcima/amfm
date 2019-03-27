// -*- faust -*- //

import("stdfaust.lib");
af = library("amfm.dsp");

tone = hslider("[1]tone[tooltip:Tone][unit:semitones]", 12, -36, 36, 1);
wet = hslider("[2]wet[tooltip:Wet gain][scale:log]", 1/sqrt(2), 0.001, 10.0, 0.1);
dry = hslider("[3]dry[tooltip:Dry gain][scale:log]", 1/sqrt(2), 0.001, 10.0, 0.1);

octaver(in) = wet*out+dry*in with {
  out = af.resynth(am, color*fm);
  color = pow(2, tone/12);
  re = af.hilbert_re(in);
  im = af.hilbert_im(in);
  am = af.am(re, im);
  fm = af.fm(re, im);
};

process(in) = octaver(in);
process_stereo(in_l, in_r) = octaver(in_l), octaver(in_r);
