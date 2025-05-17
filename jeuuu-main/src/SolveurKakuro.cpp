#include "SolveurKakuro.h"
#include "Grille.h"
#include "Case.h"
#include "UtilitaireSolveur.h"
#include <iostream>
#include <vector>
#include <set>

bool estPlacementValide(Grille* g, int i, int j, int val) {
    Case* c = g->getCase(i, j);
    if (!c || !c->estModifiable()) return false;

    auto verifierBlocCombi = [&](std::vector<Case*> bloc, const std::vector<std::set<int>>& combinaisons) {
        std::vector<int> valeurs;
        for (Case* caseBloc : bloc) {
            if (CaseVide* cv = dynamic_cast<CaseVide*>(caseBloc)) {
                valeurs.push_back((cv->getValeur() == 0) ? -1 : cv->getValeur());
            } else if (CaseFixe* cf = dynamic_cast<CaseFixe*>(caseBloc)) {
                valeurs.push_back(cf->getValeur());
            }
        }

        for (size_t k = 0; k < bloc.size(); ++k) {
            if (bloc[k] == c) {
                valeurs[k] = val;
                break;
            }
        }

        for (const auto& combinaison : combinaisons) {
            if (combinaison.size() != valeurs.size()) continue;
            std::set<int> test;
            bool ok = true;
            for (int v : valeurs) {
                if (v == -1) continue;
                if (test.count(v) || !combinaison.count(v)) {
                    ok = false;
                    break;
                }
                test.insert(v);
            }
            if (ok) return true;
        }
        return false;
    };

    CaseIndication* horizontal = nullptr;
    CaseIndication* vertical = nullptr;

    for (int jj = j - 1; jj >= 0; --jj) {
        Case* left = g->getCase(i, jj);
        if (auto ci = dynamic_cast<CaseIndication*>(left)) {
            if (ci->getSommeDroite() > 0) horizontal = ci;
            break;
        }
        if (!left || dynamic_cast<CaseNoire*>(left)) break;
    }

    for (int ii = i - 1; ii >= 0; --ii) {
        Case* up = g->getCase(ii, j);
        if (auto ci = dynamic_cast<CaseIndication*>(up)) {
            if (ci->getSommeBas() > 0) vertical = ci;
            break;
        }
        if (!up || dynamic_cast<CaseNoire*>(up)) break;
    }

    bool ok = true;
    if (horizontal)
        ok &= verifierBlocCombi(horizontal->getCasesHorizontales(), horizontal->combinaisonsValidées);
    if (vertical)
        ok &= verifierBlocCombi(vertical->getCasesVerticales(), vertical->combinaisonsValidées);
    return ok;
}

static bool verifierSolutionFinale(Grille* g) {
    for (int i = 0; i < g->getLignes(); ++i) {
        for (int j = 0; j < g->getColonnes(); ++j) {
            Case* c = g->getCase(i, j);
            if (CaseVide* cv = dynamic_cast<CaseVide*>(c)) {
                int val = cv->getValeur();
                if (val == 0 || !estPlacementValide(g, i, j, val))
                    return false;
            }
        }
    }
    return true;
}

static bool backtrack(Grille* g, const std::vector<Coord>& vides, size_t index) { // Changé de int à size_t
    if (index == vides.size())
        return verifierSolutionFinale(g);

    int i = vides[index].i;
    int j = vides[index].j;
    CaseVide* c = dynamic_cast<CaseVide*>(g->getCase(i, j));
    if (!c) return false;

    for (int val = 1; val <= 9; ++val) {
        if (estPlacementValide(g, i, j, val)) {
            c->setValeur(val);
            if (backtrack(g, vides, index + 1)) return true;
            c->setValeur(0);
        }
    }

    return false;
}

bool SolveurKakuro::resoudre(Grille* g) {
    std::vector<Coord> vides;
    trouverCasesVides(g, vides);
    g->lierBlocs();
    g->genererToutesCombinaisons();
    return backtrack(g, vides, 0);
}