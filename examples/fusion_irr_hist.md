# Irradiation History-Related Parameters for Activation Database

This document summarizes the quantities of interest for a fusion materials activation database. An underlying idea for describing the irradiation history is that neutron flux history $\phi(E,t) = f(E)\phi\hat(t)$, where $f(E)$ is a normalized flux spectrum shape, and $\phi\hat(t)$ is the respective magnitude.

## f(E): Neutron Flux Spectrum Shape

This is a neutron flux spectrum that has been integrated over all time, and normalized by particles summed over all energies. It carries units of 1/energy due to normalization of the spectrum by the corresponding energy bin width.<br>

In a fusion system, these spectra might come from the blanket, first wall, and other shielding materials.

The plot below, which represents the FNSF FW spectrum, is an example of such a spectrum shape [[1]](#1).
![alt text](https://github.com/anu1217/ALARA/blob/irr_hist/examples/flux_norm.png?raw=true)

## $t_{irr}$: Total Irradiation Time

The irradiation time $t_{irr}$ is the amount of time elapsed between the beginning and the end of the irradation period. This quantity is considered to be independent of any time-varying changes of the flux magnitude or spectrum shape.

For a typical inertial (IFE) or magnetic (MFE) confinement fusion power plant, the expected operational lifetime is ~30-40 years. However, due to the short pulse length, the irradiation time per pulse is on the order of magnitude of 1ns for IFE, and tens of ms for MFE. For an IFE plant, this means roughly 80 minutes of irradiation time over the lifetime of the plant. Other time parameters include shutdown and maintenance time on the order of 1 month.

## $\bar{\phi}$: Average Flux Magnitude Over Irradiation History

This quantity is defined as $\bar{\phi}t_{irr}={\int_{0}^{t_{irr}}\phi\hat(t)dt}$ and is determined by the irradiation times described in the previous section.

## References
<a id="1">[1]</a>
A. Davis, M. Harb, L. El-Guebaly, P. Wilson, and E. Marriott, “Neutronics aspects of the FESS-FNSF,” Fusion Engineering and Design, vol. 135, pp. 271–278, Oct. 2018, doi: https://doi.org/10.1016/j.fusengdes.2017.06.008.

