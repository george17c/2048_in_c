#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct score {
  int score;
  int high;
} sInfo;

typedef struct window {
  int yMax;
  int xMax;
  WINDOW *win;
} wInfo;

typedef struct temp {
  int score;
  int high;
} sTemp;

void generateRandom(int **m, bool delay) {
  int i, j, c;

  srand(time(NULL));

  i = rand() % 4;
  j = rand() % 4;

  // caut o pozitie libera pana gasesc
  while (m[i][j]) {
    i = rand() % 4;
    j = rand() % 4;
  }

  // adaug 2 sau 4
  m[i][j] = (rand() % 2 + 1) * 2;

  if (delay == true)
    while (true) {
      c = getch();
      if (c) break;
    }
}

void displayEyes(wInfo w) {
  char eyes[] = "0*", e;

  srand(time(NULL));

  e = eyes[rand() % strlen(eyes)];
  mvprintw(w.yMax - 5, w.xMax / 3 * 2 + 4, "%c%c", e, e);
  refresh();
}

void displayTime(wInfo w) {
  int pozx;
  pozx = w.xMax / 3 * 2 - 8;

  time_t t;
  struct tm *tminfo;
  time(&t);
  tminfo = localtime(&t);

  mvprintw(w.yMax - 10, pozx + 19, "------------");
  mvprintw(w.yMax - 9, pozx + 18, "/  %02d:%02d:%02d  \\", tminfo->tm_hour,
           tminfo->tm_min, tminfo->tm_sec);
  mvprintw(w.yMax - 8, pozx + 18, "\\ %02d.%02d.%02d /", tminfo->tm_mday,
           tminfo->tm_mon + 1, tminfo->tm_year + 1900);
  mvprintw(w.yMax - 7, pozx + 19, "------------");
  mvprintw(w.yMax - 6, pozx, "           ^__^   /");
  mvprintw(w.yMax - 5, pozx, "   _______/(oo)  /");
  mvprintw(w.yMax - 4, pozx, "/\\/(      /(__)");
  mvprintw(w.yMax - 3, pozx, "   | w----||");
  mvprintw(w.yMax - 2, pozx, "   ||     ||");

  refresh();
}

int *printValidMoves(int **m, int *validMoves, wInfo w) {
  int i, j, k = 0, nrel, cnt, valid;

  for (i = 0; i < 4; i++) {
    validMoves[i] = 0;
  }
  // [0]-sus [1]-jos [2]-dreapta [3]-stanga

  for (i = 0; i < 4; i++) {
    nrel = 0;
    for (j = 0; j < 4; j++) {
      // numar elementele de pe fiecare linie
      if (m[i][j]) nrel++;
      if ((j < 3) && m[i][j]) {
        // daca exista 2 nr egale alaturate,
        // pot muta si in dreapta si in stanga
        if (m[i][j] == m[i][j + 1]) {
          validMoves[2] = 1;
          validMoves[3] = 1;
        }
      }
    }

    if (nrel) {
      valid = 1;
      cnt = 0;
      for (j = 0; j < nrel; j++) {  // verif pt stanga
        if (m[i][j]) cnt++;
      }
      // daca toate nr sunt inghesuite in stanga, nu pot muta in stanga
      if (cnt == nrel) valid = 0;
      if (valid) validMoves[3] = 1;

      valid = 1;
      cnt = 0;
      for (j = 3; j >= (4 - nrel); j--) {  // verif pt dreapta
        if (m[i][j]) cnt++;
      }
      if (cnt == nrel) valid = 0;
      if (valid) validMoves[2] = 1;
    }
  }

  for (j = 0; j < 4; j++) {
    // aflu nr de elemente pe coloane
    nrel = 0;
    for (i = 0; i < 4; i++) {
      if (m[i][j]) nrel++;
      if ((i < 3) && m[i][j]) {
        if (m[i][j] == m[i + 1][j]) {
          validMoves[0] = 1;
          validMoves[1] = 1;
        }
      }
    }

    if (nrel) {
      valid = 1;
      cnt = 0;
      // verif pt sus
      for (i = 0; i < nrel; i++) {
        if (m[i][j]) cnt++;
      }
      if (cnt == nrel) valid = 0;
      if (valid) validMoves[0] = 1;

      valid = 1;
      cnt = 0;
      // verif pt jos
      for (i = 3; i >= (4 - nrel); i--) {
        if (m[i][j]) cnt++;
      }
      if (cnt == nrel) valid = 0;
      if (valid) validMoves[1] = 1;
    }
  }

  // si printez mutarile valide
  mvprintw(w.yMax / 3 + 5, w.xMax / 8 - 4, "Valid moves:");
  for (i = 0; i < 4; i++) {
    if (validMoves[i]) {
      if (i == 0) {
        mvprintw(w.yMax / 3 + 7 + k, w.xMax / 8 - 4, "^ up");
        k++;
      }
      if (i == 1) {
        mvprintw(w.yMax / 3 + 7 + k, w.xMax / 8 - 4, "v down");
        k++;
      }
      if (i == 2) {
        mvprintw(w.yMax / 3 + 7 + k, w.xMax / 8 - 4, "> right");
        k++;
      }
      if (i == 3) {
        mvprintw(w.yMax / 3 + 7 + k, w.xMax / 8 - 4, "< left");
        k++;
      }
    }
  }

  refresh();

  return validMoves;
}

int nrcif(int x) {
  int i = 0;
  while (x) {
    i++;
    x = x / 10;
  }
  return i;
}

void printBoard(int **m, sInfo *s, wInfo w, int autoplay, int u) {
  clear();
  int y[4], x[4], i, j, cp, select, *validMoves;
  char dir[4] = "^v><";
  validMoves = (int *)malloc(4 * sizeof(int));

  box(stdscr, 0, 0);
  displayTime(w);

  // initializez culori pentru celule
  init_color(20, 200, 600, 950);           // 2 albastru pastel
  init_color(80, 800, 800, 1000);          // 8 albastru foarte deschis
  init_color(COLOR_RED, 1000, 100, 200);   // 16
  init_color(28, 1000, 400, 100);          // 128 portocaliu
  init_color(COLOR_YELLOW, 200, 1000, 0);  // 256
  init_color(COLOR_BLUE, 100, 400, 1000);  // 512
  init_color(24, 1000, 1000, 600);         // 1024 galben pal
  init_color(48, 1000, 400, 900);          // 2048 roz

  init_pair(1, COLOR_BLACK, 20);            // pt 2
  init_pair(2, COLOR_BLACK, COLOR_GREEN);   // 4
  init_pair(3, COLOR_BLACK, 80);            // 8
  init_pair(4, COLOR_BLACK, COLOR_RED);     // 16
  init_pair(5, COLOR_BLACK, COLOR_CYAN);    // 32
  init_pair(6, COLOR_BLACK, COLOR_WHITE);   // 64
  init_pair(7, COLOR_BLACK, 28);            // 128
  init_pair(8, COLOR_BLACK, COLOR_YELLOW);  // 256
  init_pair(9, COLOR_BLACK, COLOR_BLUE);    // 512
  init_pair(10, COLOR_BLACK, 24);           // 1024
  init_pair(11, COLOR_BLACK, 48);           // 2048

  mvprintw(w.yMax / 3 - 2, w.xMax / 8 - 4, "Score: %d", s->score);
  if (s->high <= s->score) s->high = s->score;
  mvprintw(w.yMax / 3, w.xMax / 8 - 4, "Highscore: %d", s->high);
  printValidMoves(m, validMoves, w);

  mvprintw(w.yMax - 6, w.xMax / 8 - 4, "q - back to main menu");
  mvprintw(w.yMax - 5, w.xMax / 8 - 4, "r - restart");
  mvprintw(w.yMax - 4, w.xMax / 8 - 4, "p - autoplay");
  if (u) mvprintw(w.yMax - 3, w.xMax / 8 - 4, "u - undo");

  // aici sunt pozitiile celulelor
  y[0] = w.yMax / 2 - 6;
  x[0] = w.xMax / 2 - 12;
  for (i = 1; i < 4; i++) {
    y[i] = y[i - 1] + 4;
    x[i] = x[i - 1] + 7;
  }

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (m[i][j] == 0) {
        continue;
      } else {
        select = 0;
        cp = m[i][j];
        // aflu la ce putere este numarul dintr-o celula
        // in functie de asta il pot colora cum trebuie
        while (cp != 1) {
          select++;
          cp /= 2;
        }

        attron(COLOR_PAIR(select % 12));
        mvprintw(y[i] - 1, x[j], "      ");
        if (nrcif(m[i][j]) == 1)
          mvprintw(y[i], x[j], "   %d  ", m[i][j]);
        else if (nrcif(m[i][j]) == 2)
          mvprintw(y[i], x[j], "  %d  ", m[i][j]);
        else if (nrcif(m[i][j]) == 3)
          mvprintw(y[i], x[j], "  %d ", m[i][j]);
        else
          mvprintw(y[i], x[j], " %d ", m[i][j]);
        mvprintw(y[i] + 1, x[j], "      ");
        attroff(COLOR_PAIR(select % 12));
      }
    }
  }

  // daca autoplay este activ, printez acest lucru,
  // impreuna cu directia in care a mutat
  // o valoare autoplay de 10, 11, 12, 13 corespund directiilor
  // sus, jos, dreapta, stanga
  cp = autoplay;
  if ((autoplay /= 10) == 1)
    mvprintw(w.yMax / 2 - 9, w.xMax / 2 - 3, "Auto Play %c", dir[cp % 10]);

  box(w.win, 0, 0);
  wrefresh(w.win);
  refresh();

  free(validMoves);
}

void check(int **m, int *ng, wInfo w) {
  int i, j, end = 1;

  // daca exista celule egale adiacente, jocul continua
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 3; j++) {
      if (m[i][j] == m[i][j + 1]) {
        end = 0;
      }
      if (m[j][i] == m[j + 1][i]) {
        end = 0;
      }
    }
  }

  // daca se gaseste 2048 pe tabla, jucatorul castiga
  // daca exista celule goale, jocul continua
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (m[i][j] == 0) {
        if (end != 10) end = 0;
      }
      if (m[i][j] == 2048) {
        end = 10;
      }
    }
  }

  // end = 1 corespunde momentului de game over
  // end = 10 este codul pentru castig
  if (end == 1) {
    mvprintw(w.yMax / 2 - 9, w.xMax / 2 - 3, "Game Over!  ");
    // nu mai exista mutari valide, deci nu se va afisa nimic la valid moves,
    // asa ca afisez None
    mvprintw(w.yMax / 3 + 7, w.xMax / 8 - 4, "None");
    // elimin feedbackul vizual de undo si autoplay
    // aplicatia nu va accepta oricum decat q(uit) sau r(estart)
    mvprintw(w.yMax - 4, w.xMax / 8 - 4, "            ");
    mvprintw(w.yMax - 3, w.xMax / 8 - 4, "        ");
    *ng = 0;
  } else if (end == 10) {
    mvprintw(w.yMax / 2 - 10, w.xMax / 2 - 3, " You Won! ");
    mvprintw(w.yMax / 2 - 9, w.xMax / 2, ":D");
    // golesc valid moves complet, pentru ca am castigat
    mvprintw(w.yMax / 3 + 5, w.xMax / 8 - 4, "            ");
    mvprintw(w.yMax / 3 + 7, w.xMax / 8 - 4, "        ");
    mvprintw(w.yMax / 3 + 8, w.xMax / 8 - 4, "        ");
    mvprintw(w.yMax / 3 + 9, w.xMax / 8 - 4, "        ");
    mvprintw(w.yMax / 3 + 10, w.xMax / 8 - 4, "        ");
    refresh();
    *ng = 0;
  }
}

void moveCells(char where, int **m, sInfo *s, wInfo w, int *ng, int autoplay) {
  int i, j, k, q, moved, valid, merged[4][4] = {{}};

  // in continuare, m[i][j] este elementul fix, pe care vrem sa il mutam in
  // directia din variabila where, iar m[k][j] (sau m[i][k]) este elementul
  // pe care il verificam, luat din directia in care vrem sa-l mutam pe m[i][j]
  // daca este 0 putem sa mutam, iar daca sunt egale ca valoare:
  // conditia 1:
  // verific daca in patratul respectiv a mai avut loc vreo imbinare in aceeasi
  // mutare, iar daca nu, celulele se pot imbina
  // conditia 2:
  // intre cele doua patrate egale trebuie sa nu mai fie alt numar ca sa se
  // poata uni, de aceea am luat for-ul cu q

  // dupa fiecare imbinare, valoarea celulei noi se adauga la scor

  switch (where) {
    case 'u':
      moved = 0;
      for (i = 1; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          if (m[i][j]) {
            for (k = 0; k < i; k++) {
              if (m[k][j] == 0) {
                m[k][j] = m[i][j];
                m[i][j] = 0;
                moved = 1;
              } else if ((m[k][j] == m[i][j]) && !merged[k][j] &&
                         !merged[i][j]) {
                valid = 1;
                for (q = k + 1; q < i; q++) {
                  if (m[q][j]) valid = 0;
                }
                if (valid) {
                  m[k][j] *= 2;
                  m[i][j] = 0;
                  s->score += m[k][j];
                  moved = 1;
                  merged[k][j] = 1;
                }
              }
            }
          }
        }
      }
      break;

    case 'd':
      moved = 0;
      for (i = 2; i >= 0; i--) {
        for (j = 0; j < 4; j++) {
          if (m[i][j]) {
            for (k = 3; k > i; k--) {
              if (m[k][j] == 0) {
                m[k][j] = m[i][j];
                m[i][j] = 0;
                moved = 1;
              } else if ((m[k][j] == m[i][j]) && !merged[k][j] &&
                         !merged[i][j]) {
                valid = 1;
                for (q = i + 1; q < k; q++) {
                  if (m[q][j]) valid = 0;
                }
                if (valid) {
                  m[k][j] *= 2;
                  m[i][j] = 0;
                  s->score += m[k][j];
                  moved = 1;
                  merged[k][j] = 1;
                }
              }
            }
          }
        }
      }
      break;

    case 'l':
      moved = 0;
      for (i = 0; i < 4; i++) {
        for (j = 1; j < 4; j++) {
          if (m[i][j]) {
            for (k = 0; k < j; k++) {
              if (m[i][k] == 0) {
                m[i][k] = m[i][j];
                m[i][j] = 0;
                moved = 1;
              } else if ((m[i][k] == m[i][j]) && !merged[i][k] &&
                         !merged[i][j]) {
                valid = 1;
                for (q = k + 1; q < j; q++) {
                  if (m[i][q]) valid = 0;
                }
                if (valid) {
                  m[i][k] *= 2;
                  m[i][j] = 0;
                  s->score += m[i][k];
                  moved = 1;
                  merged[i][k] = 1;
                }
              }
            }
          }
        }
      }
      break;

    case 'r':
      moved = 0;
      for (i = 0; i < 4; i++) {
        for (j = 2; j >= 0; j--) {
          if (m[i][j]) {
            for (k = 3; k > j; k--) {
              if (m[i][k] == 0) {
                m[i][k] = m[i][j];
                m[i][j] = 0;
                moved = 1;
              } else if ((m[i][k] == m[i][j]) && !merged[i][k] &&
                         !merged[i][j]) {
                valid = 1;
                for (q = j + 1; q < k; q++) {
                  if (m[i][q]) valid = 0;
                }
                if (valid) {
                  m[i][k] *= 2;
                  m[i][j] = 0;
                  s->score += m[i][k];
                  moved = 1;
                  merged[i][k] = 1;
                }
              }
            }
          }
        }
      }
      break;

    default:
      break;
  }

  if (moved) {
    printBoard(m, s, w, autoplay, 1);
    displayEyes(w);

    generateRandom(m, true);
    printBoard(m, s, w, autoplay, 1);
    check(m, ng, w);
  }
}

void autoPlay(int **m, sInfo *s, wInfo w, int *ng) {
  int *validMoves, i, j, k, l, maxMerges = 0, merges = 0, finalMove;
  int paired[4][4] = {{}}, cnt1, cnt2, ok = 1;

  validMoves = (int *)malloc(4 * sizeof(int));
  printValidMoves(m, validMoves, w);
  // [0]-sus [1]-jos [2]-dreapta [3]-stanga

  // setez o mutare valida implicita,
  // in caz ca nu se poate face nicio imbinare
  for (k = 0; k < 4; k++)
    if (validMoves[k]) finalMove = k;

  // numar cate imbinari se pot face pt sus + jos
  if (validMoves[0] && validMoves[1]) {
    for (j = 0; j < 4; j++) {
      for (i = 0; i < 4; i++) {
        // imi fixez un element nenul
        // si verific daca se poate imbina cu cele din dreapta lui
        if (m[i][j]) {
          for (l = i; l < 4; l++) {
            if ((m[l][j]) && (l > i)) {
              if ((m[l][j] == m[i][j]) && !paired[i][j] && !paired[l][j]) {
                merges++;
                paired[i][j] = 1;
                paired[l][j] = 1;
              } else
                break;
            }
          }
        }
      }
    }
  }
  if (merges > maxMerges) {
    maxMerges = merges;
    finalMove = 0;
  }

  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      paired[i][j] = 0;
    }
  }

  // numar cate imbinari se pot face pt dreapta + stanga
  if (validMoves[2] && validMoves[3]) {
    merges = 0;
    for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
        if (m[i][j]) {
          for (l = j; l < 4; l++) {
            if ((m[i][l]) && l > j) {
              if ((m[i][l] == m[i][j]) && !paired[i][j] && !paired[i][l]) {
                merges++;
                paired[i][j] = 1;
                paired[i][l] = 1;
              } else
                break;
            }
          }
        }
      }
    }
    if (merges > maxMerges) {
      maxMerges = merges;
      finalMove = 2;
    }
  }

  // daca nu putem imbina nimic,
  // pentru ca algoritmul de mai sus tinde sa organizeze tabla
  // ca o tabla de sah, cu elemente egale pe diagonala, am facut urmatoarea
  // optimizare: in situatii in care exista doua numere egale care se afla
  // diagonal unul fata de celalalt, care se pot uni doar daca mut in stanga
  // si apoi in sus/jos, o voi face
  if (maxMerges == 0)
    for (i = 0; i < 3; i++) {
      cnt1 = 0;
      cnt2 = 0;
      // numar elementele nule de pe linia i de la stanga
      // pana la primul element nenul
      for (j = 0; j < 3; j++) {
        if (m[i][j] == 0)
          cnt1++;
        else
          break;
      }
      // si de pe linia i + 1
      for (j = 0; j < 4; j++) {
        if (m[i + 1][j] == 0)
          cnt2++;
        else
          break;
      }

      for (k = cnt1; k < 3; k++) {
        // daca exista un element care are doar 0-uri in stanga (sau e primul
        // pe linie) si linia urmatoare are cu 1 zero in plus (adica pot muta
        // in stanga) si daca gasesc elemente egale diagonal, voi face acea
        // mutare
        if ((m[i][cnt1]) && (cnt2 == cnt1 + 1))
          if (m[i][k] == m[i + 1][k + 1]) {
            moveCells('l', m, s, w, ng, 13);
            ok = 0;
          }
      }
    }

  if (ok) {
    char dir[4] = "udrl";
    moveCells(dir[finalMove], m, s, w, ng, (10 + finalMove));
  }
}

void cpy(int **a, int **b) {
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      a[i][j] = b[i][j];
    }
  }
}

void updateTmp(int **m, int **tmp, sInfo *s, sTemp *temp, int *u) {
  cpy(tmp, m);
  temp->score = s->score;
  if (temp->high != s->high) temp->high = s->high;
  *u = 1;
}

char play(char status, int **m, int **tmp, sInfo *s, wInfo *w, int *ng) {
  // ^ tabla initiala / tabla resume
  clear();

  int i, j, errTime = 0, u = 0, ok = 1;
  int autoStart = 150;
  // pentru ca folosesc halfdelay(2), o valoare autostart = 150 corespunde cu
  // 30s de inactivitate
  char c;
  sTemp temp;

  // la new game golim tabla si generam doua valori aleatoare 2 sau 4
  if (status == 'n') {
    for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
        m[i][j] = 0;
      }
    }

    generateRandom(m, false);
    generateRandom(m, false);

    s->score = 0;
  }
  // daca statusul este r(esume), jocul revine la ce a memorat inainte
  // fara alte instructiuni

  printBoard(m, s, *w, 0, u);
  cpy(tmp, m);
  temp.high = s->high;

  while (true) {
    ok = 1;

    // in momentul cand pierzi / castigi se asteapta doar q sau r
    if (*ng == 0)
      while (true) {
        displayTime(*w);
        c = getch();
        if (c == 'q')
          return c;
        else if (c == 'r') {
          *ng = 1;
          return 'R';
        }
      }

    c = getch();

    if (c == ERR) {
      // in caz ca redimensionam terminalul si programul nu se afla
      // in autoPlay, actualizam ecranul (in autoPlay
      // se apeleaza printBoard automat)
      getmaxyx(stdscr, w->yMax, w->xMax);
      w->win = newwin(17, 31, w->yMax / 2 - 8, w->xMax / 2 - 14);
      if (errTime < autoStart - 6) printBoard(m, s, *w, 0, u);
      errTime++;
    }
    if (errTime == autoStart) {
      //  actualizez matricea tmp si scorul precedent
      updateTmp(m, tmp, s, &temp, &u);
      // si jocul incepe sa se joace singur
      autoPlay(m, s, *w, ng);
      // durata dintre mutari este de 1.2s (fara a lua in calul
      // delay-ul de la celulele noi)
      errTime = autoStart - 6;
    }

    displayTime(*w);

    if ((c == 'u') && u) {
      errTime = 0;
      cpy(m, tmp);
      s->score = temp.score;
      if (temp.high != s->high) s->high = temp.high;
      printBoard(m, s, *w, 0, u);
      // ochii vacutei nu se schimba decat daca putem da undo
      if (u) displayEyes(*w);
      u = 0;
    }

    switch (c) {
      case 'p':
        mvprintw(w->yMax / 2 - 9, w->xMax / 2 - 7, "Starting Auto Play");
        //  pornim autoPlay dupa 1s
        for (i = 0; i < 5; i++) {
          displayTime(*w);
          c = getch();
          if (c != ERR) {
            mvprintw(w->yMax / 2 - 9, w->xMax / 2 - 7, "                  ");
            ok = 0;
            break;
          }
        }
        if (ok) {
          updateTmp(m, tmp, s, &temp, &u);
          errTime = autoStart;
          autoPlay(m, s, *w, ng);
          errTime = autoStart - 6;
        }
        break;

      case 'q':
        return c;

      case 'r':
        *ng = 1;
        return 'R';

      case 'A':
      case 'w':
        errTime = 0;
        updateTmp(m, tmp, s, &temp, &u);
        moveCells('u', m, s, *w, ng, 0);
        break;

      case 'B':
      case 's':
        errTime = 0;
        updateTmp(m, tmp, s, &temp, &u);
        moveCells('d', m, s, *w, ng, 0);
        break;

      case 'C':
      case 'd':
        errTime = 0;
        updateTmp(m, tmp, s, &temp, &u);
        moveCells('r', m, s, *w, ng, 0);
        break;

      case 'D':
      case 'a':
        errTime = 0;
        updateTmp(m, tmp, s, &temp, &u);
        moveCells('l', m, s, *w, ng, 0);
        break;

      default:
        continue;
    }
  }
}

char mainMenu(int *ng, wInfo *w) {
  noecho();
  halfdelay(2);

  init_pair(1, COLOR_YELLOW, COLOR_BLACK);

  clear();

  char c;
  int i, resumeFail = 0, yN = 18, yR = 20, yQ = 22;

  box(stdscr, 0, 0);

  attron(COLOR_PAIR(1));
  mvprintw(7, w->xMax / 2 - 15, "    ____   ___    __   ___  ");
  mvprintw(8, w->xMax / 2 - 15, "   (___ \\ / _ \\  /  | /   \\ ");
  mvprintw(9, w->xMax / 2 - 15, "     __) ) | | |/ O |_\\ O / ");
  mvprintw(10, w->xMax / 2 - 15, "    / __/| | | /__   _) _ \\ ");
  mvprintw(11, w->xMax / 2 - 15, "   | |___| |_| |  | |( (_) )");
  mvprintw(12, w->xMax / 2 - 15, "   |_____)\\___/   |_| \\___/");
  attroff(COLOR_PAIR(1));

  // cursorul va selecta 'New Game' cand nu e niciun joc in desfasurare
  if (*ng == 0) {
    attron(A_REVERSE);
    mvprintw(yN, w->xMax / 2 - 4, "New Game");
    attroff(A_REVERSE);
    mvprintw(yR, w->xMax / 2 - 3, "Resume");
    mvprintw(yQ, w->xMax / 2 - 2, "Quit");

    attron(A_BLINK);
    mvprintw(yN, w->xMax / 2 + 5, "<");
    mvprintw(yN, w->xMax / 2 - 6, ">");
    attroff(A_BLINK);

    i = yN;
  } else {
    // in caz contrar va selecta optiunea 'Resume'
    mvprintw(yN, w->xMax / 2 - 4, "New Game");
    attron(A_REVERSE);
    mvprintw(yR, w->xMax / 2 - 3, "Resume");
    attroff(A_REVERSE);
    mvprintw(yQ, w->xMax / 2 - 2, "Quit");

    attron(A_BLINK);
    mvprintw(yR, w->xMax / 2 + 4, "<");
    mvprintw(yR, w->xMax / 2 - 5, ">");
    attroff(A_BLINK);

    i = yR;
  }

  while (true) {
    curs_set(0);

    c = getch();
    if (c == ERR) {
      clear();
      // resumeFail devine 1 si incepe sa creasca (am scris mai jos) daca am
      // vrut sa dau resume fara niciun joc in desfasurare si afisez un mesaj
      // corespunzator timp de 3s (resumeFail creste o data la 20ms)
      if (resumeFail && (resumeFail <= 15)) {
        mvprintw(25, w->xMax / 2 - 13, "There is no game in progress");
      } else {
        resumeFail = 0;
      }

      box(stdscr, 0, 0);
      getmaxyx(stdscr, w->yMax, w->xMax);
      w->win = newwin(17, 31, w->yMax / 2 - 8, w->xMax / 2 - 14);

      attron(COLOR_PAIR(1));
      mvprintw(7, w->xMax / 2 - 15, "    ____   ___    __   ___  ");
      mvprintw(8, w->xMax / 2 - 15, "   (___ \\ / _ \\  /  | /   \\ ");
      mvprintw(9, w->xMax / 2 - 15, "     __) ) | | |/ O |_\\ O / ");
      mvprintw(10, w->xMax / 2 - 15, "    / __/| | | /__   _) _ \\ ");
      mvprintw(11, w->xMax / 2 - 15, "   | |___| |_| |  | |( (_) )");
      mvprintw(12, w->xMax / 2 - 15, "   |_____)\\___/   |_| \\___/");
      attroff(COLOR_PAIR(1));
      refresh();
    }

    if (c == 'A') {
      if (i == yN)
        i += 4;
      else
        i -= 2;
    } else if (c == 'B') {
      if (i == yQ)
        i -= 4;
      else
        i += 2;
    }

    // selectia optiunilor din meniu
    // 'New Game'
    if (i == yN) {
      // sterg mesajul de la resume daca intru in alta optiune
      if (resumeFail) {
        mvprintw(25, w->xMax / 2 - 13, "                            ");
        resumeFail = 0;
      }

      attron(A_REVERSE);
      mvprintw(yN, w->xMax / 2 - 4, "New Game");
      attroff(A_REVERSE);
      mvprintw(yR, w->xMax / 2 - 3, "Resume");
      mvprintw(yQ, w->xMax / 2 - 2, "Quit");

      attron(A_BLINK);
      mvprintw(yN, w->xMax / 2 + 5, "<");
      mvprintw(yN, w->xMax / 2 - 6, ">");
      attroff(A_BLINK);

      mvprintw(yR, w->xMax / 2 - 5, " ");
      mvprintw(yR, w->xMax / 2 + 4, " ");

      mvprintw(yQ, w->xMax / 2 - 4, " ");
      mvprintw(yQ, w->xMax / 2 + 3, " ");

      // 'Resume
    } else if (i == yR) {
      mvprintw(yN, w->xMax / 2 - 4, "New Game");
      attron(A_REVERSE);
      mvprintw(yR, w->xMax / 2 - 3, "Resume");
      attroff(A_REVERSE);
      mvprintw(yQ, w->xMax / 2 - 2, "Quit");

      mvprintw(yN, w->xMax / 2 + 5, " ");
      mvprintw(yN, w->xMax / 2 - 6, " ");

      attron(A_BLINK);
      mvprintw(yR, w->xMax / 2 + 4, "<");
      mvprintw(yR, w->xMax / 2 - 5, ">");
      attroff(A_BLINK);

      mvprintw(yQ, w->xMax / 2 - 4, " ");
      mvprintw(yQ, w->xMax / 2 + 3, " ");

      // 'Quit'
    } else if (i == yQ) {
      if (resumeFail) {
        mvprintw(25, w->xMax / 2 - 13, "                            ");
        resumeFail = 0;
      }

      mvprintw(yN, w->xMax / 2 - 4, "New Game");
      mvprintw(yR, w->xMax / 2 - 3, "Resume");
      attron(A_REVERSE);
      mvprintw(yQ, w->xMax / 2 - 2, "Quit");
      attroff(A_REVERSE);

      mvprintw(yN, w->xMax / 2 + 5, " ");
      mvprintw(yN, w->xMax / 2 - 6, " ");

      mvprintw(yR, w->xMax / 2 - 5, " ");
      mvprintw(yR, w->xMax / 2 + 4, " ");

      attron(A_BLINK);
      mvprintw(yQ, w->xMax / 2 + 3, "<");
      mvprintw(yQ, w->xMax / 2 - 4, ">");
      attroff(A_BLINK);
    }

    if ((i == yN) && (c == 10)) {
      *ng = 1;
      return 'n';
    }

    if ((i == yR) && (c == 10)) {
      if (*ng == 1)
        return 'r';
      else {
        mvprintw(25, w->xMax / 2 - 13, "There is no game in progress");
        resumeFail = 1;
      }
    }
    if (resumeFail) resumeFail++;

    if ((i == yQ) && (c == 10)) {
      curs_set(1);
      mvprintw(25, w->xMax / 2 - 16, "Are you sure you want to quit? Y/n ");
      while (true) {
        char q = getch();
        if ((q == 10) || (q == 'y') || (q == 'Y')) {
          return 'q';
        } else if (q == 'n') {
          int j;
          for (j = (w->xMax / 2 - 16); j < (w->xMax / 2 + 19); j++) {
            mvprintw(25, j, " ");
          }
          break;
        } else
          continue;
      }
    }
  }
}

int main() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  start_color();
  raw();

  char opt1, opt2;
  int **m, **tmp, i, ng = 0;
  sInfo s;
  wInfo w;

  FILE *file = fopen("highscore.in", "r+");
  fscanf(file, "%d", &s.high);

  m = (int **)malloc(4 * sizeof(int *));
  tmp = (int **)malloc(4 * sizeof(int *));
  for (i = 0; i < 4; i++) {
    m[i] = (int *)malloc(4 * sizeof(int));
    tmp[i] = (int *)malloc(4 * sizeof(int));
  }

  while (true) {
    getmaxyx(stdscr, w.yMax, w.xMax);
    w.win = newwin(17, 31, w.yMax / 2 - 8, w.xMax / 2 - 14);
    //          height width    startY         startX

    // mainMenu intoarce n(ewgame), r(esume) sau q(quit)
    opt1 = mainMenu(&ng, &w);
    if (opt1 == 'n')
      opt2 = play('n', m, tmp, &s, &w, &ng);
    else if (opt1 == 'r')
      opt2 = play('r', m, tmp, &s, &w, &ng);
    else if (opt1 == 'q')
      break;

    // r - resume   R - restart

    // cand sunt in ecranul de joc (in cadrul functiei play), pot intoarce
    // doar n(ewgame), R(estart) sau q(uit)
    if (opt2 == 'R') opt2 = play('n', m, tmp, &s, &w, &ng);
    while (opt2 != 'q') {
      opt2 = play('n', m, tmp, &s, &w, &ng);
    }
  }

  // eliberam memoria
  for (i = 0; i < 4; i++) {
    free(m[i]);
    free(tmp[i]);
  }
  free(m);
  free(tmp);

  // actualizam highscore-ul
  fseek(file, 0, SEEK_SET);
  fprintf(file, "%d", s.high);
  fclose(file);

  endwin();
  return 0;
}
