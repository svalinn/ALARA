

/* Of the following 4 bits, all combinations with !TRUNCEOS lead to
   continue.  Of the remaining 8 combinations, only 6 are allowed
   due to the restriction that !TRUNCC implies !IGNORC.  Those 6
   combinations lead to only 3 states:
   IGNORE: Only ignore if fail ignore test always: 15
   TRUNCATE_STABLE: Anything => 8 will truncate stables AT LEAST
   TRUNCATE: Anything => 12 will be truncated always.

   Note: We are allowing truncation and ignoring of radioactives.

  */
   
#define TRUNCEOS  8
#define TRUNCC    4
#define IGNOREOS  2
#define IGNORC    1

#define IGNORE          15
#define TRUNCATE_STABLE  8
#define TRUNCATE        12

#define CONTINUE         0
#define SOLVE            1
#define SOLVED           2
#define FINISHED_ROOT    3
