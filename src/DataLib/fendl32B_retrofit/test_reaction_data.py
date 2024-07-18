import reaction_data

def test_count_emitted_particles():

    emitted_particle_string = "np"

    particle = 'n'
    exp = 1
    obs = reaction_data.count_emitted_particles(particle, emitted_particle_string)

    assert obs == exp

    particle = 'p'
    exp = 1
    obs = reaction_data.count_emitted_particles(particle, emitted_particle_string)

    assert obs == exp

    particle = 't'
    exp = 0
    obs = reaction_data.count_emitted_particles(particle, emitted_particle_string)

    assert obs == exp


    emitted_particle_string = "357n14d"

    particle = 'n'
    exp = 357
    obs = reaction_data.count_emitted_particles(particle, emitted_particle_string)

    assert obs == exp

    particle = 'p'
    exp = 0
    obs = reaction_data.count_emitted_particles(particle, emitted_particle_string)

    assert obs == exp
    
    particle = 'd'
    exp = 14
    obs = reaction_data.count_emitted_particles(particle, emitted_particle_string)

    assert obs == exp
    