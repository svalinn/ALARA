import string

def make_template_obj():

    template_string = """geometry rectangular
$volume_placeholder
$mat_loading_placeholder
$mixtures_placeholder

material_lib $material_lib
element_lib $nuclib
data_library alaralib $alaralib

output zone
    number_density
end

#     flux name    fluxin file   norm   shift   unused
flux  my_flux     $flux_file $norm     0      default

schedule    my_schedule``
    $schedule_time $schedule_units my_flux my_pulse_history 0  s
end
pulsehistory  my_pulse_history
    $num_pulses    0.0    s
end

#other parameters
truncation $trunc_tol
dump_file $dump_filepath
    """

    template_obj = string.Template(template_string)
    return template_obj
