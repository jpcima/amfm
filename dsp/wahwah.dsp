// -*- faust -*- //

import("stdfaust.lib");
af = library("amfm.dsp");

lforate = hslider("lforate[tooltip:LFO rate][unit:Hz]", 0.5, 0.1, 5.0, 0.01);
lfodepth = hslider("lfodepth[tooltip:LFO depth]", 0.8, 0.0, 1.0, 0.01);
wet = hslider("wet[tooltip:Wet gain][scale:log]", 1/sqrt(2), 0.001, 10.0, 0.1);
dry = hslider("dry[tooltip:Dry gain][scale:log]", 1/sqrt(2), 0.001, 10.0, 0.1);

wahwah(in) = wet*out+dry*in with {
  out = af.resynth(am, fmmod);

  wah = 2*lfodepth*os.lf_triangle(lforate);

  fg = 0.1*pow(4, wah);
  ffc = (450.0/ma.SR) * pow(2, 2.3*wah);
  fq = pow(2, 2*(1-wah)+1);

  rho = 1-ma.PI*ffc/fq;
  theta = 2*ma.PI*ffc;
  a1 = -2*rho*cos(theta);
  a2 = rho*rho;

  fmmod = fg*fm-a1*fm'-a2*fm'';

  re = af.hilbert_re(in);
  im = af.hilbert_im(in);
  am = af.am(re, im);
  fm = af.fm(re, im);
};

process(in) = wahwah(in);
process_stereo(in_l, in_r) = wahwah(in_l), wahwah(in_r);
