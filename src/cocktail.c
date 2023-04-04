/*
 * Playground+ - cocktail.c
 * Cocktail recipe generator for PG+
 * (c) 2023 by Raindog (Jeremy Modjeska)
 * Updated 2023.03.19
 * https://github.com/jmodjeska/pgplus-cocktail/
 * ---------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>

#ifdef USING_DMALLOC
#include <dmalloc.h>
#endif

#include "include/config.h"
#include "include/player.h"
#include "include/fix.h"
#include "include/proto.h"

#define CLEAR(x) memset( x, '\0', sizeof(x) )

/* strcmp for comparing pointers - used for alpha sort */
int pstrcmp(const void *p1, const void *p2)
{
  ENTERFUNCTION;
  int c = strcmp(p1, p2);
  EXITFUNCTION;
  return c;
}

/* downcase a string */
void downcase(char *str)
{
  ENTERFUNCTION;
  int i;
  for ( i = 0; str[i]; i++ ) str[i] = tolower(str[i]);
  EXITFUNCTION;
}

void cocktail(player * p, char *str)
{
  char *oldstack = stack;
  ENTERFUNCTION;

  /* Validate user age for US legal compliance */
  if ( get_age(p) < 21 )
  {
    tell_player(p, " Sorry, you must be 21 years old to use this command.\n");
    EXITFUNCTION;
    return;
  }

  /* Validate cocktail DB exists */
  FILE *fp = fopen("files/cocktails","r");
  if (!fp)
  {
    tell_player(p,
        " Sorry, can't find the cocktail database (error logged).\n");
    log("error", "Couldn't find files/cocktails.");
    EXITFUNCTION;
    return;
  }

  /* Validate user input */
  if (!*str)
  {
    tell_player(p, " Format: cocktail <drink>\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

  /* Setup vars */
  char r_title[75] = "Cocktail recipe for: ";
  char vowels[5] = "aeiou";
  char recipe[4096];          // Recipe content
  char desc[512];             // Description string
  char t[512];                // Temp (current file line)
  char tdown[512];            // Temp (current file line, downcased)
  char drinks[500][32];       // List of drinks
  char row[75] = "  ";        // Row length (for list)
  int r_mode = 0;             // Mode: 0: Search; 1: Capture; 2: List
  int r_count = 0, d = 0;     // Results and drinks counters
  int line_num = 1;
  int d_length = sizeof(drinks)/sizeof(drinks[0]);
  for ( d = 0; d < d_length; d++ ) { CLEAR(drinks[d]); }

  /* Clear buffers to prevent corrupted output */
  CLEAR(recipe);
  CLEAR(drinks);

  /* Setup search string */
  downcase(str);
  char search_key[512];
  sprintf(search_key, "name: %s", str);

  /* Config for list mode */
  if ( strcmp(str, "list" ) == 0 )
  {
    r_mode = 2;
    strncpy(r_title, "Available drink recipes", 23);
  }

  /* Main loop - search and capture recipe data */
  while ( fgets(t, 512, fp) != NULL )
  {
    if ( r_count >= d_length )
    {
      log("error", "List of drinks in files/cocktails exceeds array max.");
      break;
    }

    /* Strip newlines and create downcase variant of current line */
    t[strcspn(t, "\n")] = 0;
    strcpy(tdown, t);
    downcase(tdown);

    /* List mode: capture drink names only */
    if ( (r_mode == 2) && (strstr(t, "name:")) )
    {
      memmove(tdown, tdown+6, strlen(tdown));
      strcpy(drinks[r_count], tdown);
      r_count++;
    }

    /* Capture mode: If a match was previously found, continue capturing
       results until the last line of the recipe */
    else if ( r_mode == 1 )
    {
      if ( strstr(t, "timing:") )
      {
        char an[2] = "A";
        memmove(tdown, tdown+8, strlen(tdown));
        if ( strchr(vowels, tdown[0]) != NULL ) strcpy(an, "An");
        sprintf(desc, "\n%s %s ", an, tdown);
        strcat(recipe, desc);
      }
      else if ( strstr(t, "taste:") )
      {
        memmove(tdown, tdown+7, strlen(tdown));
        sprintf(desc, "drink with a %s taste.\n\n", tdown);
        strcat(recipe, desc);
      }
      else if ( strstr(t, "ingredients:" ) )
      {
        strcat(recipe, "Ingredients:\n\n");
      }
      else if ( strstr(t, "ingredient:" ) )
      {
        memmove(t, t+12, strlen(t));
        t[0] = toupper(t[0]);
        sprintf(desc, "- %s: ", t);
        strcat(recipe, desc);
      }
      else if ( strstr(t, "amount:" ) )
      {
        memmove(t, t+8, strlen(t));
        strcat(recipe, t);
      }
      else if ( strstr(t, "unit:" ) )
      {
        memmove(t, t+6, strlen(t));
        sprintf(desc, " %s\n", t);
        strcat(recipe, desc);
      }
      else if ( strstr(t, "preparation:" ) )
      {
        t[0] = toupper(t[0]);
        sprintf(desc, "\n%s.", t);
        strcat(recipe, desc);
        r_mode = 0;
      }
    }

    /* Search mode: Compare the search string to the current line to find
       "name: <drink>". If match found, enable further capturing (Capture mode)
       and set the title */
    else if ( (r_mode == 0) && (strcmp(tdown, search_key) == 0) )
    {
      memmove(t, t+6, strlen(t));
      sprintf(recipe, "%s: ", t);
      strcat(r_title, t);
      r_count++;
      r_mode = 1;
    }

    /* Clear some char arrays and move to next line of the DB file */
    CLEAR(t);
    CLEAR(tdown);
    line_num++;
  }

  if (fp) fclose(fp);

  /* Handle unmatched search string */
  if ( r_count == 0 )
  {
    tell_player(p, " Couldn't find a recipe for that drink.\n");
    EXITFUNCTION;
    return;
  }

  /* Begin output */
  pstack_mid(p, r_title);

  /* List mode: format and show the cocktail list */
  /* Alpha sort the drinks list */
  qsort(drinks, d_length, sizeof(*drinks), pstrcmp);

  if ( r_mode == 2 )
  {
    stack += sprintf(stack, "\n");
    for ( d = 0; d <= d_length; d++ )
    {
      /* Skip empty elements */
      if ( strlen(drinks[d]) < 1 ) continue;

      /* Append drink to row */
      strcat(row, drinks[d]);

      /* Comma-separate drinks */
      if ( d < (d_length-1) ) strcat(row, ", ");

      /* Fancy line wrapping */
      if ( (strlen(row) > 55) || (d == (d_length-1)) )
      {
        stack += sprintf(stack, "%s\n", row);
        strcpy(row, "  ");
      }
    }
    stack += sprintf(stack, "\n  There are %d recipes available.", r_count);
  }

  /* Tell player what their cocktail recipe is */
  else {
    stack += sprintf(stack, "\n%s", recipe);
  }

  /* Wrap up output */
  pstack_bot(p, "");
  stack = end_string(stack);
  tell_player(p, oldstack);
  stack = oldstack;

  CLEAR(recipe);
  CLEAR(drinks);
  EXITFUNCTION;
}
