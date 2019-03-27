// -*- faust -*- //

import("stdfaust.lib");
af = library("amfm.dsp");

detune = hslider("detune[tooltip:Detune]", 0.001, 0.0001, 0.01, 0.0005);
wet = hslider("wet[tooltip:Wet gain][scale:log]", 1/sqrt(2), 0.001, 10.0, 0.1);
dry = hslider("dry[tooltip:Dry gain][scale:log]", 1/sqrt(2), 0.001, 10.0, 0.1);

chorus(in) = wet*out+dry*in with {
  out = linegain * (out_nd + (par(i, ndelays, out_d(i)) :> +));

  out_nd = af.resynth(am, fm);
  out_d(i) = af.resynth(am, delayed(fm, i)*(1+detune*detunefactor(i)));

  ndelays = 4;
  linegain = 1.0/(ndelays+1);

  maxdeltime = deltime(0);
  deltime(0) = 0.020453515;
  deltime(1) = 0.020000000;
  deltime(2) = 0.019546485;
  deltime(3) = 0.019727891;

  detunefactor(0) = -2;
  detunefactor(1) = -1;
  detunefactor(2) = 1;
  detunefactor(3) = 2;

  delayed(x, i) = de.fdelay(deltime(i)*ma.SR, deltime(i)*ma.SR, x);

  re = af.hilbert_re(in);
  im = af.hilbert_im(in);
  am = af.am(re, im);
  fm = af.fm(re, im);
};

process(in) = chorus(in);
process_stereo(in_l, in_r) = chorus(in_l), chorus(in_r);
