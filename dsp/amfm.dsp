// -*- faust -*- //

import("stdfaust.lib");

resynth(am, fm) = am*os.osccos(fm/ma.SR);

am(re, im) = sqrt(re*re+im*im);

fm(re, im) = k*f with {
  f = unwrap(ph-ph');
  k = ma.SR/(2*ma.PI);
  ph = atan2(im, re);
  unwrap(ph) = ba.if(ph>=ma.PI, ph-2*ma.PI, ba.if(ph<ma.neg(ma.PI), ph+2*ma.PI, ph));
};

amfm(re, im) = am(re, im), fm(re, im);

hilbert_re = seq(i, 6, _hilbert_stage(c(i))) with {
  c(0) = 1.2524;
  c(1) = 5.5671;
  c(2) = 22.3423;
  c(3) = 89.6271;
  c(4) = 364.7914;
  c(5) = 2770.1114;
};

hilbert_im = seq(i, 6, _hilbert_stage(c(i))) with {
  c(0) = 0.3609;
  c(1) = 2.7412;
  c(2) = 11.1573;
  c(3) = 44.7581;
  c(4) = 179.6242;
  c(5) = 798.4578;
};

hilbert = _ <: hilbert_re, hilbert_im;

_hilbert_stage(p, x) = x:(+:*(k):+(x'))~ma.neg with {
  k = _hilbert_coef(p);
};

_hilbert_coef(p) = -beta with {
  polefreq = p*15;
  rc = 1/(2*ma.PI*polefreq);
  alpha = (1/rc)/(2*ma.SR);
  beta = (1-alpha)/(1+alpha);
};
