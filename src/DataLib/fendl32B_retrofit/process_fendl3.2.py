# Import packages
import reaction_data as rxd

def main():
    """
    Main method when run as a command line script.
    """

    mt_dict = rxd.process_mt_data(rxd.load_mt_table('mt_table.csv'))

if __name__ == '__main__':
    main()