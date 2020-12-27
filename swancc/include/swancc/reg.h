/* reg.h - registers for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_REG_H
#define _SWANCC_SWANCC_REG_H

/*
 * The compiler generates "addresses" of the form
 *     indirect(indcount) (rx + offset)
 *
 * where:
 *     rx is a machine register (possibly null)
 *     n is the indirection count (possibly 0)
 *     offset is a constant.
 *
 * It does not support more complicated formats like
 *     indirect(indcount) (rx + index * scale + offset).
 *
 * The register is coded as bit flag in the  storage  component of
 * the symbol structure. This allows groups of registers to be tested
 * using bitwise "&". Throughout the compiler, the group of these bit
 * flags has the type  reg_t. If there are only a few registers, reg_t
 * can be an uint8_t. It must be unsigned if the high bit is
 * used, to avoid sign extension problems. For bootstrapping the compiler
 * from a compiler with no uint8_t, the unsigned type should be
 * used instead (with a signifigant waste of storage).
 *
 * The bit flags should really be defined as ((reg_t) whatever) but
 * then they would not (necessarily) be constant expressions and couldn't
 * be used in switch selectors or (worse) preprocessor expressions.
 *
 * The CONSTANT and GLOBAL (non-) register bits are almost
 * equivalent. A constant with nonzero indirection is marked as a
 * GLOBAL and not a CONSTANT. This makes it easier to test for a constant
 * CONSTANT. Globals which are formed in this way are converted to
 * constants if their indirection count is reset to 0 (by & operator).
 */

/* register bit flags */
typedef enum strage_t
{
    NOSTORAGE = 0x000U,    /* structure/union member offsets */
    CONSTANT  = 0x001U,    /* offsets are values */
    BREG      = 0x002U,
    DREG      = 0x004U,
    INDREG0   = 0x008U,
    INDREG1   = 0x010U,
    INDREG2   = 0x020U,
    LOCAL     = 0x040U,
    GLOBAL    = 0x080U,    /* offsets from storage name or 0 */
    CCREG     = CONSTANT,  /* arg to PSHS/PULS functions only */

#ifdef FRAMEPOINTER
    FRAMEREG = LOCAL,
    STACKREG = 0x100U,
    DATREG1  = 0x200U,
    DATREG2  = 0x400U,
    DATREG1B = 0x800U,
#endif
} store_t;

/* data for pushing and pulling registers */
#define MINREGCHAR 'A'
#define FLAGSREGCHAR 'f'
#define pushchar() pushlist(AXREG)

/* special registers */
#define ALREG    BREG
#define AXREG    DREG
#define DXREG    DATREG2
#define MULREG   DATREG1B
#define SHIFTREG DATREG1B

/* groups of registers */
#define ALLDATREGS (BREG|DREG)
#define CHARREGS BREG
#define MAXREGS 1           /* number of data registers */
#define WORKDATREGS (BREG|DREG)

/* function call and return registers */
#define ARGREG RETURNREG    /* for (1st) argument */
#define LONGARGREGS LONGRETURNREGS /* for long or float arg */
#define LONGRETURNREGS (INDREG0|LONGREG2)
#define LONGREG2 DREG

#define LONGRETSPECIAL    /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
#define RETURNREG DREG

/* registers to be used by software operations */
#define OPREG INDREG0        /* 2nd reg for software ops (1st is DREG) */
#define OPWORKREG INDREG2    /* 3rd register for software ops */

/* maximum indirection count for 1 instruction */
#define MAXINDIRECT 1

#endif /* _SWANCC_SWANCC_REG_H */
