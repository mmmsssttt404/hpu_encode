#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINE 256

typedef enum {
    INST_INVALID = 0,
    INST_PADD,
    INST_PSUB,
    INST_PMUL,
    INST_PMAC,
    INST_PMOV,
    INST_PBCAST,
    INST_PNTT,
    INST_PINTT,
    INST_PTWLD,
    INST_PTWID,
    INST_PTWI2,
    INST_PSHCFG,
    INST_PSHUF,
    INST_PSHUF2,
    INST_PSEED,
    INST_PSAMPLE,
    INST_PMODLD,
    INST_PMODSW,
    INST_SLOAD,
    INST_SSTORE
} inst_type_t;

typedef struct {
    inst_type_t type;
    char mnemonic[32];

    int prs1;
    int prs2;
    int prd;

    int pcst;
    int ptw;
    int pshf;
    int pseedid;
    int pmod;
    int saddr;
} parsed_inst_t;

static void trim(char *s) {
    if (!s) return;

    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    int n = (int)strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static void init_inst(parsed_inst_t *inst) {
    memset(inst, 0, sizeof(*inst));
    inst->type = INST_INVALID;
    inst->prs1 = -1;
    inst->prs2 = -1;
    inst->prd = -1;
    inst->pcst = -1;
    inst->ptw = -1;
    inst->pshf = -1;
    inst->pseedid = -1;
    inst->pmod = -1;
    inst->saddr = -1;
}

static const char *type_name(inst_type_t t) {
    switch (t) {
        case INST_PADD:    return "PADD";
        case INST_PSUB:    return "PSUB";
        case INST_PMUL:    return "PMUL";
        case INST_PMAC:    return "PMAC";
        case INST_PMOV:    return "PMOV";
        case INST_PBCAST:  return "PBCAST";
        case INST_PNTT:    return "PNTT";
        case INST_PINTT:   return "PINTT";
        case INST_PTWLD:   return "PTWLD";
        case INST_PTWID:   return "PTWID";
        case INST_PTWI2:   return "PTWI2";
        case INST_PSHCFG:  return "PSHCFG";
        case INST_PSHUF:   return "PSHUF";
        case INST_PSHUF2:  return "PSHUF2";
        case INST_PSEED:   return "PSEED";
        case INST_PSAMPLE: return "PSAMPLE";
        case INST_PMODLD:  return "PMODLD";
        case INST_PMODSW:  return "PMODSW";
        case INST_SLOAD:   return "SLOAD";
        case INST_SSTORE:  return "SSTORE";
        default:           return "INVALID";
    }
}

/* 支持 prd/prs1/prs2 统一写成 p<number> */
static int parse_preg(const char *s, int *out) {
    if (sscanf(s, "p%d", out) == 1) return 0;
    return -1;
}

/* 支持 pcst 写成 c<number> */
static int parse_pcst(const char *s, int *out) {
    if (sscanf(s, "c%d", out) == 1) return 0;
    return -1;
}

/* 支持 ptw/pshf/pseedid/pmod 用纯数字 */
static int parse_int(const char *s, int *out) {
    char *end = NULL;
    long v = strtol(s, &end, 0);
    if (end == s || *end != '\0') return -1;
    *out = (int)v;
    return 0;
}

static int parse_line(const char *line_in, parsed_inst_t *out) {
    if (!line_in || !out) return -1;

    char line[MAX_LINE];
    char a[64], b[64], c[64];
    snprintf(line, sizeof(line), "%s", line_in);
    trim(line);

    init_inst(out);

    if (line[0] == '\0') return -1;

    /* padd prs1, prs2, prd */
    if (strncmp(line, "padd ", 5) == 0) {
        if (sscanf(line, "padd %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PADD;
            strcpy(out->mnemonic, "padd");
            return 0;
        }
    }

    /* psub prs1, prs2, prd */
    if (strncmp(line, "psub ", 5) == 0) {
        if (sscanf(line, "psub %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PSUB;
            strcpy(out->mnemonic, "psub");
            return 0;
        }
    }

    /* pmul prs1, prs2, prd */
    if (strncmp(line, "pmul ", 5) == 0) {
        if (sscanf(line, "pmul %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PMUL;
            strcpy(out->mnemonic, "pmul");
            return 0;
        }
    }

    /* pmac prs1, prs2, prd */
    if (strncmp(line, "pmac ", 5) == 0) {
        if (sscanf(line, "pmac %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PMAC;
            strcpy(out->mnemonic, "pmac");
            return 0;
        }
    }

    /* pmov prs1, prd */
    if (strncmp(line, "pmov ", 5) == 0) {
        if (sscanf(line, "pmov %63[^,], %63s", a, b) == 2 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prd) == 0) {
            out->type = INST_PMOV;
            strcpy(out->mnemonic, "pmov");
            return 0;
        }
    }

    /* pbcast pcst, prd */
    if (strncmp(line, "pbcast ", 7) == 0) {
        if (sscanf(line, "pbcast %63[^,], %63s", a, b) == 2 &&
            parse_pcst(a, &out->pcst) == 0 &&
            parse_preg(b, &out->prd) == 0) {
            out->type = INST_PBCAST;
            strcpy(out->mnemonic, "pbcast");
            return 0;
        }
    }

    /* pntt prs1, prs2, prd */
    if (strncmp(line, "pntt ", 5) == 0) {
        if (sscanf(line, "pntt %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PNTT;
            strcpy(out->mnemonic, "pntt");
            return 0;
        }
    }

    /* pintt prs1, prs2, prd */
    if (strncmp(line, "pintt ", 6) == 0) {
        if (sscanf(line, "pintt %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PINTT;
            strcpy(out->mnemonic, "pintt");
            return 0;
        }
    }

    /* ptwld ptw */
    if (strncmp(line, "ptwld ", 6) == 0) {
        if (sscanf(line, "ptwld %63s", a) == 1 &&
            parse_int(a, &out->ptw) == 0) {
            out->type = INST_PTWLD;
            strcpy(out->mnemonic, "ptwld");
            return 0;
        }
    }

    /* ptwid */
    if (strcmp(line, "ptwid") == 0) {
        out->type = INST_PTWID;
        strcpy(out->mnemonic, "ptwid");
        return 0;
    }

    /* ptwi2 */
    if (strcmp(line, "ptwi2") == 0) {
        out->type = INST_PTWI2;
        strcpy(out->mnemonic, "ptwi2");
        return 0;
    }

    /* pshcfg pshf */
    if (strncmp(line, "pshcfg ", 7) == 0) {
        if (sscanf(line, "pshcfg %63s", a) == 1 &&
            parse_int(a, &out->pshf) == 0) {
            out->type = INST_PSHCFG;
            strcpy(out->mnemonic, "pshcfg");
            return 0;
        }
    }

    /* pshuf prs1, prd */
    if (strncmp(line, "pshuf ", 6) == 0) {
        if (sscanf(line, "pshuf %63[^,], %63s", a, b) == 2 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prd) == 0) {
            out->type = INST_PSHUF;
            strcpy(out->mnemonic, "pshuf");
            return 0;
        }
    }

    /* pshuf2 prs1, prs2, prd */
    if (strncmp(line, "pshuf2 ", 7) == 0) {
        if (sscanf(line, "pshuf2 %63[^,], %63[^,], %63s", a, b, c) == 3 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_preg(b, &out->prs2) == 0 &&
            parse_preg(c, &out->prd) == 0) {
            out->type = INST_PSHUF2;
            strcpy(out->mnemonic, "pshuf2");
            return 0;
        }
    }

    /* pseed pseedid */
    if (strncmp(line, "pseed ", 6) == 0) {
        if (sscanf(line, "pseed %63s", a) == 1 &&
            parse_int(a, &out->pseedid) == 0) {
            out->type = INST_PSEED;
            strcpy(out->mnemonic, "pseed");
            return 0;
        }
    }

    /* psample prd */
    if (strncmp(line, "psample ", 8) == 0) {
        if (sscanf(line, "psample %63s", a) == 1 &&
            parse_preg(a, &out->prd) == 0) {
            out->type = INST_PSAMPLE;
            strcpy(out->mnemonic, "psample");
            return 0;
        }
    }

    /* pmodld pmod */
    if (strncmp(line, "pmodld ", 7) == 0) {
        if (sscanf(line, "pmodld %63s", a) == 1 &&
            parse_int(a, &out->pmod) == 0) {
            out->type = INST_PMODLD;
            strcpy(out->mnemonic, "pmodld");
            return 0;
        }
    }

    /* pmodsw pmod */
    if (strncmp(line, "pmodsw ", 7) == 0) {
        if (sscanf(line, "pmodsw %63s", a) == 1 &&
            parse_int(a, &out->pmod) == 0) {
            out->type = INST_PMODSW;
            strcpy(out->mnemonic, "pmodsw");
            return 0;
        }
    }

    /* sload saddr, prd */
    if (strncmp(line, "sload ", 6) == 0) {
        if (sscanf(line, "sload %63[^,], %63s", a, b) == 2 &&
            parse_int(a, &out->saddr) == 0 &&
            parse_preg(b, &out->prd) == 0) {
            out->type = INST_SLOAD;
            strcpy(out->mnemonic, "sload");
            return 0;
        }
    }

    /* sstore prs1, saddr */
    if (strncmp(line, "sstore ", 7) == 0) {
        if (sscanf(line, "sstore %63[^,], %63s", a, b) == 2 &&
            parse_preg(a, &out->prs1) == 0 &&
            parse_int(b, &out->saddr) == 0) {
            out->type = INST_SSTORE;
            strcpy(out->mnemonic, "sstore");
            return 0;
        }
    }

    return -1;
}

static void print_inst(const parsed_inst_t *inst) {
    printf("type     : %s\n", type_name(inst->type));
    printf("mnemonic : %s\n", inst->mnemonic);

    switch (inst->type) {
        case INST_PADD:
        case INST_PSUB:
        case INST_PMUL:
        case INST_PMAC:
        case INST_PNTT:
        case INST_PINTT:
        case INST_PSHUF2:
            printf("prs1     : p%d\n", inst->prs1);
            printf("prs2     : p%d\n", inst->prs2);
            printf("prd      : p%d\n", inst->prd);
            break;

        case INST_PMOV:
        case INST_PSHUF:
            printf("prs1     : p%d\n", inst->prs1);
            printf("prd      : p%d\n", inst->prd);
            break;

        case INST_PBCAST:
            printf("pcst     : c%d\n", inst->pcst);
            printf("prd      : p%d\n", inst->prd);
            break;

        case INST_PTWLD:
            printf("ptw      : %d\n", inst->ptw);
            break;

        case INST_PTWID:
        case INST_PTWI2:
            break;

        case INST_PSHCFG:
            printf("pshf     : %d\n", inst->pshf);
            break;

        case INST_PSEED:
            printf("pseedid  : %d\n", inst->pseedid);
            break;

        case INST_PSAMPLE:
            printf("prd      : p%d\n", inst->prd);
            break;

        case INST_PMODLD:
        case INST_PMODSW:
            printf("pmod     : %d\n", inst->pmod);
            break;

        case INST_SLOAD:
            printf("saddr    : %d\n", inst->saddr);
            printf("prd      : p%d\n", inst->prd);
            break;

        case INST_SSTORE:
            printf("prs1     : p%d\n", inst->prs1);
            printf("saddr    : %d\n", inst->saddr);
            break;

        default:
            break;
    }
}

int main(void) {
    char line[MAX_LINE];
    parsed_inst_t inst;

    printf("Input one instruction per line. Type 'quit' to exit.\n");

    while (1) {
        printf("\nasm> ");
        if (!fgets(line, sizeof(line), stdin)) break;

        trim(line);
        if (strcmp(line, "quit") == 0) break;

        if (parse_line(line, &inst) == 0) {
            print_inst(&inst);
        } else {
            printf("Parse error: unsupported or invalid instruction format.\n");
        }
    }

    return 0;
}