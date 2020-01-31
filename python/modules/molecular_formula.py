
import re
from collections import Counter

from modules import atom_table


class MolecularFormula(object):

    _FORMULA_RESIDUE_ORDER = ['C', '(13)C', 'H', 'D', 'Br', 'Cl', 'N',
                              '(15)N', 'O', '(18)O', 'P', 'S', 'Se']

    def __init__(self, seq='', n_term=True, c_term=True):
        self.seq = seq
        self.formula = self._seq_to_counter(seq, n_term=n_term, c_term=c_term)


    def __str__(self):
        ret = ''

        # Keep track of already printed AAs
        printed = {aa: False for aa in self.formula.keys()}
        for aa in self._FORMULA_RESIDUE_ORDER:
            if self.formula[aa] == 0:
                printed[aa] = True
                continue
            if self.formula[aa] == 1:
                ret += aa
            else:
                ret += '{}{}'.format(aa, self.formula[aa])
            printed[aa] = True

        for aa, p in printed.items():
            if not p:
                ret += '{}{}'.format(aa, self.formula[aa])

        return ret


    def __repr__(self):
        return str('MolecularFormula({})'.format(dict(self.formula)))


    @staticmethod
    def _seq_to_counter(seq, n_term=True, c_term=True):
        '''
        Convert `seq` to formula.

        Paramaters
        ----------
        seq: str
            Amino acid sequence as string.
        n_term: str
            Should n terminus be added to formula?
        c_term: str
            Should c terminus be added to formula?

        Returns
        -------
        formula: Counter
            Counter of atoms in formula.
        '''

        formula = Counter()
        for c in seq:
            try:
                formula += atom_table.BASE_RESIDUE_ATOMS[c]
            except KeyError:
                raise KeyError('Unknown amino acid in sequence: {}, {}'.format(seq, c.upper()))

        if seq != '':
            if n_term:
                formula += atom_table.BASE_RESIDUE_ATOMS['N_TERM']
            if c_term:
                formula += atom_table.BASE_RESIDUE_ATOMS['C_TERM']

        return formula


    def set_seq(self, seq):
        '''
        Reset peptide sequence and formula.

        Paramaters
        ----------
        seq: str
            Peptide sequence to add.
        '''

        self.seq = seq
        self.formula = self._seq_to_counter(seq)


    def add_mod(self, name, residue):
        '''
        Add residue modificaton to formula.

        Paramaters
        ----------
        name: str
            Name of modification.
        residue: str
            Single letter aa code on which modification occured.

        Raises
        ------

        '''

        try:
            temp_mod = atom_table.MODIFICATIONS[name.lower()][residue.upper()]
        except KeyError:
            raise KeyError('Unknown modification: {}, for residue: {}'.format(name, residue))

        self.formula.update(temp_mod)


    def __iadd__(self, rhs):
        '''
        Add another MolecularFormula. seq member is unmodified.

        Paramaters
        ----------
        rhs: MolecularFormula
            Another formula to add.
        '''
        self.formula += rhs.formula
        return self

