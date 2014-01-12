
#include	<stdio.h>
#include	"tass.h"


tass_select_help() {
	char title[100];

	sprintf(title, "%s, Group Selection Commands", TASS_HEADER);

	ClearScreen();
	center_line(0, title);
	fputs("\r\n\r\n", stdout);

	printf("\t4\tSelect group 4\r\n");
	printf("\t^D^F\tHalf/Full Page down\r\n");
	printf("\t^R\tReset .newsrc\r\n");
	printf("\t^U^B\tHalf/Full Page up\r\n");
	printf("\t^K\tDelete group\r\n");
	printf("\t^Y\tUndelete group\r\n");
	printf("\t<CR>\tRead current group\r\n");
	printf("\t<TAB>\tView next unread group\r\n");
	printf("\tc\tMark group as all read\r\n");
	printf("\tg\tChoose a new group by name\r\n");
	printf("\tj\tDown a line\r\n");
	printf("\tk\tUp a line\r\n");
	printf("\tq\tQuit\r\n");
	printf("\ts\tSubscribe to current group\r\n");
	printf("\tu\tUnsubscribe to current group\r\n");
	printf("\ty\tYank in groups that are not in the .newsrc\r\n");
	printf("\t$\tReread group list from .newsrc\r\n");
	printf("\t/?\tGroup search forward (?=backward)\r\n");

	center_line(LINES, "-- hit any key --");
	ReadCh();
}


tass_group_help() {
	char title[100];

	sprintf(title, "%s, Index Page Commands", TASS_HEADER);
	ClearScreen();
	center_line(0, title);
	fputs("\r\n\r\n", stdout);

	printf("\t4\tSelect article 4\r\n");
	printf("\t<CR>\tRead current article\r\n");
	printf("\t<TAB>\tView next unread article or group\r\n");
	printf("\t^D^F\tHalf/Full Page down\r\n");
	printf("\t^U^B\tHalf/Full Page up\r\n");
	printf("\taA\tAuthor search forward (A=backward)\r\n");
#ifdef XTRA
	printf("\tB\tSearch article bodies for string\r\n");
#endif
	printf("\tc\tMark all articles as read\r\n");
	printf("\tg\tChoose a new group by name\r\n");
	printf("\tjk\tDown (k=up) a line\r\n");
	printf("\tK\tMark thread as read & advance\r\n");
	printf("\tnp\tGo to next (p=previous) group\r\n");
	printf("\tNP\tGo to next (P=previous) unread article\r\n");
	printf("\tq\tQuit\r\n");
	printf("\tsu\tSubscribe (u=unsubscribe) to this group\r\n");
	printf("\tt\tReturn to group selection index\r\n");
	printf("\tw\tPost an article\r\n");
	printf("\t/?\tSearch forward (?=backward) for subject\r\n");
	printf("\t-\tShow last article\r\n");

	center_line(LINES, "-- hit any key --");
	ReadCh();
}


tass_page_help() {
	char title[100];

	sprintf(title, "%s, Article Pager Commands", TASS_HEADER);

	ClearScreen();
	center_line(0, title);
	fputs("\r\n\r\n", stdout);

	printf("\t4\tRead response 4 in this thread (0 is basenote)\r\n");
	printf("\t<CR>\tSkip to next base article\r\n");
	printf("\t<TAB>\tAdvance to next page or unread article\r\n");
	printf("\taA\tAuthor search forward (A=backward)\r\n");
	printf("\t<>\tScroll page left/right\r\n");
	printf("\tb\tBack a page\r\n");
#ifdef XTRA
	printf("\tB\tSearch article bodies for string\r\n");
#endif
	printf("\tC\tCancel this article\r\n");
	printf("\tfF\tPost a followup (F=include text)\r\n");
	printf("\twW\tPost and cc a followup (W = include text)\r\n");
	printf("\tH\tShow article headers\r\n");
	printf("\ti\tReturn to index page\r\n");
printf("\tkK\tMark article (K=thread) as read & advance to next unread\r\n");
	printf("\tmM\tMail/bounce this article to someone\r\n");
	printf("\tnN/pP\tSkip to the next/previous (N/P=unread) article)\r\n");
	printf("\trR\tReply through mail (R=include text) to author\r\n");
	printf("\tsS\tSave article (S=thread) to file\r\n");
	printf("\tt\tReturn to group selection page\r\n");
	printf("\tz\tMark article as unread\r\n");
	printf("\t^R\tRedisplay first page of article\r\n");
	printf("\t%%, ^X\tToggle rot-13 decoding for this article\r\n");
	printf("\t-\tShow last article\r\n");
	printf("\t|\tPipe article into command\r\n");

	page_cont();
}

