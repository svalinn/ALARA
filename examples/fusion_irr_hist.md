# Irradiation History-Related Parameters for Activation Database

## This document summarizes the quantities of interest for a fusion materials activation database. An underlying idea for describing the irradiation history is that neutron flux history $\phi(E,t) = f(E)\phi\hat(t)$, where $f(E)$ is a normalized flux spectrum shape, and $\phi\hat(t)$ is the respective magnitude.

## f(E): Neutron Flux Spectrum Shape

This is a neutron flux spectrum that has been integrated over all time, and normalized by particles summed over all energies. It carries units of 1/energy due to normalization of the spectrum by the corresponding energy bin width.<br>

In a fusion system, these spectra might come from the blanket, FW, and other shielding materials.

The plot below, which represents the FNSF FW spectrum, is an example of such a spectrum shape.<br>

![alt text](https://github.com/anu1217/ALARA/blob/irr_hist/examples/flux_norm.png?raw=true)

## $\bar{\phi}$: Average Flux Magnitude Over Irradiation History

This quantity is defined as $\frac{{\int_{0}^{t_{irr}}\phi\hat(t)}}{{t_{irr}}}$ and is determined by the irradiation times described in the next section.

## Total Irradiation Time

For a typical inertial (IFE) or magnetic (MFE) confinement fusion power plant, the expected operational lifetime is ~30-40 years. However, due to the short pulse length, the irradiation time per pulse is on the order of magnitude of 1ns for IFE, and tens of ms for MFE. For an IFE plant, this means roughly 80 minutes of irradiation time over the lifetime of the plant. Other time parameters include shutdown and maintenance time on the order of 1 month.




