// -*- faust -*- //

import("stdfaust.lib");
af = library("amfm.dsp");

mutator(in1, in2) = out with {
  mix = hslider("mix12[tooltip:Mix 1-2]", 0.5, 0.0, 1.0, 0.01);

  out = af.resynth(am1, fmmix);

  re1 = af.hilbert_re(in1);
  im1 = af.hilbert_im(in1);
  am1 = af.am(re1, im1);
  fm1 = af.fm(re1, im1);

  re2 = af.hilbert_re(in2);
  im2 = af.hilbert_im(in2);
  fm2 = af.fm(re2, im2);

  fmmix = (1-mix)*fm1+mix*fm2;
};

process(in1, in2) = mutator(in1, in2);
process_stereo(in1_l, in1_r, in2_l, in2_r) = mutator(in1_l, in2_l), mutator(in1_r, in2_r);
