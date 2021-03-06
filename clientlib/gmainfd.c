/**
 * @file
 * @brief Implements a gmainloop source for reading file descriptor pipes
 * @details This class implements a base class for reading file descriptor pipes and stashing the
 * results away in strings.  It is notable that this class is <i>not</i> a subclass of @ref AssimObj.
*
 *
 * @author Copyright &copy; 2013 - Alan Robertson <alanr@unix.sh>
 * @n
 *  This file is part of the Assimilation Project.
 *  The Assimilation software is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The Assimilation software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the Assimilation Project software.  If not, see http://www.gnu.org/licenses/
 */

#include <projectcommon.h>
#include <memory.h>
#include <gmainfd.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#	ifdef HAVE_FCNTL_H
#		include <fcntl.h>
#	endif
#endif

#define READBUFSIZE	1024

///@defgroup GmainFd GmainFd class.
/// (base) Class for reading from file descriptors (usually pipes) from gmainloop programs.
///@{
///@ingroup C_Classes


FSTATIC gboolean gmainfd_gsource_prepare(GSource* source, gint* timeout);
FSTATIC gboolean gmainfd_gsource_check(GSource* source);
FSTATIC gboolean gmainfd_gsource_dispatch(GSource* source, GSourceFunc callback, gpointer user_data);
FSTATIC gboolean gmainfd_gsource_readmore(GMainFd* self);
FSTATIC void     gmainfd_gsource_finalize(GSource* source);

static GSourceFuncs gmainfd_source_funcs = {
	gmainfd_gsource_prepare,
	gmainfd_gsource_check,
	gmainfd_gsource_dispatch,
	gmainfd_gsource_finalize,
	NULL,
	NULL
};

FSTATIC void gmainfd_newtext(GMainFd*, const char *, int len);
FSTATIC void gmainfd_finalize(GMainFd*);

/// Construct a new @ref GMainFd object and return it.
GMainFd*
gmainfd_new(gsize cpsize, int fd, int priority, GMainContext* context)
{
	GSource*	source;
	GMainFd*	self;

	if (cpsize < sizeof(GMainFd)) {
		cpsize = sizeof(GMainFd);
	}
	source = g_source_new(&gmainfd_source_funcs, cpsize);
	g_return_val_if_fail(source != NULL, NULL);
	proj_class_register_object(source, "GSource");
	self = NEWSUBCLASS(GMainFd, source);

	self->textread = NULL;
	self->finalize = NULL;
	self->newtext = gmainfd_newtext;
	self->readmore = gmainfd_gsource_readmore;
	memset(&self->gfd, 0, sizeof(self->gfd));


	/* Now initialize all the gmainloop stuff */
	self->gfd.fd = fd;
	self->gfd.events = G_IO_IN|G_IO_ERR|G_IO_HUP;
	self->gfd.revents = 0;
#ifdef HAVE_FCNTL
	{
		int	fdflags;
		fdflags = fcntl(fd, F_GETFL);
		if (-1 == fdflags) {
			g_warning("%s.%d: fcntl(%d, F_GETFL) failed errno=%d [%s]"
			,	__FUNCTION__, __LINE__, fd, errno, g_strerror(errno));
		}else{
			fdflags |= O_NONBLOCK;
			if (fcntl(fd, F_SETFL, fdflags) < 0) {
				g_warning("%s.%d: fcntl(%d, F_SETFL, 0x%x) failed) errno=%d [%s]"
				,	__FUNCTION__, __LINE__, fd, fdflags
				,	errno, g_strerror(errno));
			}
		}
	}
#endif
	g_source_add_poll(source, &self->gfd);
	g_source_set_priority(source, priority);
	self->gsourceid = g_source_attach(source, context);
	if (self->gsourceid == 0) {
		g_source_remove_poll(source, &self->gfd);
		memset(self, 0, sizeof(*self));
		g_source_unref(source); // Should cause proj_class_dissociate(source) to be called
		source = NULL;
		self = NULL;
	}
	return self;
}

/// Just stash away our new string - appending to what's already there
FSTATIC void
gmainfd_newtext(GMainFd* self, const char * string, int len)
{
	if (self->textread) {
		self->textread = g_string_append_len(self->textread, string, len);
	}else{
		self->textread = g_string_new_len(string, len);
	}
}


/// @ref GmainFd version of a gmainloop prepare function - get ready to go into the poll function
FSTATIC gboolean
gmainfd_gsource_prepare(GSource* dummysource, gint* dummytimeout)
{
	// Don't need to do anything prior to a poll(2) call...
	(void)dummysource;
	(void)dummytimeout;
	return FALSE;
}

/// @ref GmainFd version of a gmainloop check function - check for input after the poll function
FSTATIC gboolean
gmainfd_gsource_check(GSource* source)
{
	GMainFd*	self = CASTTOCLASS(GMainFd, source);
	// revents: received events...
	if (self->gfd.revents & G_IO_HUP) {
		g_source_remove_poll(source, &self->gfd);
		// The other end of the pipe was closed
		self->gfd.events = 0;
		self->atEOF = TRUE; // is this right?
	}
	if (self->gfd.revents & G_IO_ERR) {
		g_warning("%s.%d: received I/O error on file descriptor %d"
		,	__FUNCTION__, __LINE__, self->gfd.fd);
		g_source_remove_poll(source, &self->gfd);
		self->gfd.events = 0;
		self->atEOF = TRUE; // is this right?
	}
	return 0 != self->gfd.revents;
}

/// @ref GmainFd version of a gmainloop dispatch function - we read the data from the file descriptor
FSTATIC gboolean
gmainfd_gsource_dispatch(GSource* source, GSourceFunc unusedcallback, gpointer unused_user_data)
{
	GMainFd*	self = CASTTOCLASS(GMainFd, source);

	(void)unusedcallback;
	(void)unused_user_data;
	return gmainfd_gsource_readmore(self);
}

FSTATIC gboolean
gmainfd_gsource_readmore(GMainFd* self)
{
	char		readbuf[READBUFSIZE];
	int		readrc;

	while ((readrc = read(self->gfd.fd, &readbuf, sizeof(readbuf))) > 0) {
		self->newtext(self, readbuf, readrc);
	}
	// End of File?
	if (0 == readrc) {
		self->atEOF = TRUE;
		self->gfd.events = 0;
	}
	return !self->atEOF && self->gfd.events != 0;
}

/// @ref GmainFd version of a gmainloop finalize function
FSTATIC void
gmainfd_gsource_finalize(GSource* source)
{
	GMainFd*	self = CASTTOCLASS(GMainFd, source);

	if (self->finalize) {
		self->finalize(self);
	}
	if (self->textread) {
		g_string_free(self->textread, TRUE);
		self->textread = NULL;
	}
	if (self->gfd.fd >= 0) {
		close(self->gfd.fd);
		self->gfd.fd = -1;
	}
	proj_class_dissociate(source);
	source = NULL;
	self = NULL;
}
///@}
