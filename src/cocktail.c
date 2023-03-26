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

#define CLEAR(x) memset( x, '\0', 512 )

void cocktail(player * p, char *str)
{
  char *oldstack = stack;
  ENTERFUNCTION;

  /* Validate user age for US legal compliance */
  if ( get_age(p) < 21 ) {
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

  /* Downcase search string and remove apostrophes */
  int i;
  for ( i = 0; str[i]; i++ )
  {
    str[i] = tolower(str[i]);
  }

  /* Setup vars */
  char r_title[75] = "Cocktail recipe for: ";
  char vowels[5] = "aeiou";
  char recipe[4096];         // Recipe content
  char desc[512];            // Description string
  char t[512];               // Temp (current file line)
  char tdown[512];           // Temp (current file line, downcased)
  char drinks[500][32];      // List of drinks
  int r_flag = 0;            // Flag: 0: search; 1: matched; 2: list
  int line_num = 1;
  int find_result = 0;

  /* Setup search string */
  char search_key[512];
  sprintf(search_key, "name: %s", str);

  /* Config for list mode */
  if ( strcmp(str, "list" ) == 0 )
  {
    r_flag = 2;
    strncpy(r_title, "Available drink recipes", 23);
  }

  /* Search the recipe DB for the drink name */
  while ( fgets(t, 512, fp) != NULL)
  {
    /* Create downcase variant of current line; strip newlines */
    strcpy(tdown, t);
    for ( i = 0; t[i]; i++ )
    {
      tdown[i] = tolower(tdown[i]);
      tdown[strcspn(tdown, "\n")] = 0;
      t[strcspn(t, "\n")] = 0;
    }

    /* List mode: capture drink names only */
    if ( (r_flag == 2) && (strstr(t, "name:")) )
    {
      memmove(tdown, tdown+6, strlen(tdown));
      strcpy(drinks[find_result], tdown);
      find_result++;
    }

    /* If a match was previously found, continue capturing results
       until the last line of the recipe */
    else if ( r_flag == 1 )
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
        r_flag = 0;
        break;
      }
    }

    /* Compare search string to current line to find "name: <drink>"
       If match found, enable further capturing and set the title */
    else if ( (r_flag == 0) && (strcmp(tdown, search_key) == 0) )
    {
      memmove(t, t+6, strlen(t));
      sprintf(recipe, "%s: ", t);
      strcat(r_title, t);
      find_result++;
      r_flag = 1;
    }

    line_num++;
  }

  if ( find_result == 0 )
  {
    tell_player(p, " Couldn't find a recipe for that drink.\n");
    if (fp) fclose(fp);
    EXITFUNCTION;
    return;
  }

  if (fp) fclose(fp);

  pstack_mid(p, r_title);

  /* List mode: format and show the cocktail list */
  char row[75] = "  ";

  if ( r_flag == 2 )
  {
    stack += sprintf(stack, "\n");
    for ( i = 0; i < find_result; i++ )
    {
      strcat(row, drinks[i]);
      if ( i < (find_result - 1) ) strcat(row, ", ");

      if ( (strlen(row) > 55) || (i == (find_result-1)) ) {
        stack += sprintf(stack, "%s\n", row);
        strcpy(row, "  ");
      }
    }
    stack += sprintf(stack, "\n  There are %d recipes available.", find_result);
  }

  /* Tell player what their cocktail recipe is */
  else {
    stack += sprintf(stack, "\n%s", recipe);
  }

  pstack_bot(p, "");
  stack = end_string(stack);
  tell_player(p, oldstack);
  stack = oldstack;

  CLEAR(t);
  CLEAR(tdown);
  CLEAR(recipe);
  EXITFUNCTION;
}
