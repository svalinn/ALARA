
# Import packages
import pytest
import reaction_data as rxd
from numpy import array

@pytest.mark.parametrize(
        "(particle, emitted_particle_string, exp)",
    [
        ('n'      , 'np'                   , 1  ) ,
        ('p'      , 'np'                   , 1  ) ,
        ('t'      , 'np'                   , 0  ) ,
        ('n'      , '357n14d'              , 357) ,
        ('p'      , '357n14d'              , 0  ) ,
        ('d'      , '357n14d'              , 14 ) ,
        ('3He'    , '3He8α'                , 1  ) ,
        ('α'      , '3He8α'                , 8  )
    ]
)
def test_count_emitted_particles(particle, emitted_particle_string, exp):
    obs = rxd.count_emitted_particles(particle, emitted_particle_string)
    assert obs == exp

@pytest.mark.parametrize(
    "(emitted_particles, exp)",
    [
        ('np' , {'n' : 1, 'p' : 1}),
        ('2d3t3He', {'d' : 2, 't' : 3, '3He' : 1}),
        ('total', {}),
        ('α75γn3', {'α' : 1, 'γ': 75, 'n': 1})
        ('Xα', {})
    ]
)
def test_emission_breakdown(emitted_particles, exp):
    obs = rxd.emission_breakdown(emitted_particles)
    assert obs == exp


@pytest.mark.parametrize(
    "(emission_dict, exp)",
    [
        ({'n' : 1 , 'p' : 1} , array([ 1 , -1])),
        ({'t' : 2 , 'n' : 1, '3He' : 3} , array([-7 , -8])),
        ({'α' : 1 , 'γ' : 4, 'd': 1} , array([-2 , -3])) ,
        ({'n' : 1 , 'p' : 1, 'd' : 1 , 't' : 1 ,
          '3He' : 1 , 'α' : 1 ,'γ' : 1} , array([-6 , -7])),
        ({}, array([None, None]))
    ]
)
def test_nucleon_changes(emission_dict, exp):
    obs = rxd.nucleon_changes(emission_dict)
    assert obs == exp

