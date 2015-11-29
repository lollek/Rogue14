#ifndef ROGUE14_DEATH_H
#define ROGUE14_DEATH_H

/* Reasons for player dying */
enum death_reason
{
  DEATH_UNKNOWN,
  DEATH_ARROW,
  DEATH_BOLT,
  DEATH_DART,
  DEATH_FLAME,
  DEATH_ICE,
  DEATH_HUNGER
};

/* Return a string describing the death */
char* death_reason(char buf[], int reason);

/* Handle player death */
void death(int monst) __attribute__((noreturn));

#endif /* ROGUE14_DEATH_H */