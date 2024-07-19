
# Import packages
import pytest
import reaction_data as rxd

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
    "(mt_data_csv, exp)",
    [
        ('mt_test1.csv', {'11'                 : 
                          {'Reaction'          :   '(z,2nd)'   ,
                           'delKZA'            :      -10030   ,
                           'Emitted Particles' :       '2nd'
                           }})                                 ,
        ('mt_test2.csv', {'1'                  :
                          {'Reaction'          : '(n,total)'   ,
                           'delKZA'            :       'N/A'   , 
                           'Emitted Particles' :     'total' 
                           }})                                 ,
        ('mt_test3.csv', {'30'                 :
                          {'Reaction'          :  '(z,2n2α)'   ,
                           'delKZA'            :      -40090   ,
                           'Emitted Particles' :      '2n2α'
                           }})                                 ,
        ('mt_test4.csv', {'106'                : 
                          {'Reaction'          :   '(z,3He)'   ,
                           'delKZA'            :      -20020   ,
                           'Emitted Particles' :       '3He'
                           }})                                 ,
        ('mt_test5.csv', {'207'                :
                          {'Reaction'          :    '(z,Xα)'   ,
                           'delKZA'            :       'N/A'   ,
                           'Emitted Particles' :        'Xα'   
                          }})                                  ,
        ('mt_test6.csv', {'700'                :
                          {'Reaction'          :    '(z,t1)'   ,
                           'delKZA'            :      -20019   ,
                           'Emitted Particles' :          't'
                           }})
    ]
)
def test_process_mt_data(mt_data_csv, exp):
    dir = './files_for_tests'
    obs = rxd.process_mt_data(rxd.load_mt_table(f'{dir}/{mt_data_csv}'))
    assert obs == exp
