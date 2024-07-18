
# Import packages
import pytest
import reaction_data as rxd

@pytest.mark.parametrize(
    "particle, emitted_particle_string, exp",
    [
        ('n', 'np', 1),
        ('p', 'np', 1),
        ('t', 'np', 0),
        ('n', '357n14d', 357),
        ('p', '357n14d', 0),
        ('d', '357n14d', 14),
        ('3He', '3He8α', 1),
        ('α', '3He8α', 8)
    ]
)
def test_count_emitted_particles(particle, emitted_particle_string, exp):
    obs = rxd.count_emitted_particles(particle, emitted_particle_string)
    assert obs == exp