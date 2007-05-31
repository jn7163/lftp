/*
 * lftp and utils
 *
 * Copyright (c) 2007 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Id$ */

#include <config.h>
#include <string.h>
#include "xstring.h"
#include "trio.h"

void xstring::get_space(size_t s,size_t g)
{
   if(!buf)
      buf=(char*)xmalloc(size=s+1);
   else if(size<s+1)
      buf=(char*)realloc(buf,size=(s|(g-1))+1);
   else if(size>=g*8 && s<size/2)
      buf=(char*)realloc(buf,size/=2);
   buf[s]=0;
}

void xstring::init(const char *s,int len)
{
   init();
   if(s)
      nset(s,len);
}
void xstring::init(const char *s)
{
   init();
   if(s)
      set(s);
}

const char *xstring::nset(const char *s,int len)
{
   if(!s)
   {
      xfree(buf);
      init();
      return 0;
   }
   this->len=len;
   if(s==buf)
      return buf;
   if(s>buf && s<buf+size)
      return (char*)memmove(buf,s,len);
   get_space(len);
   return (char*)memcpy(buf,s,len);
}
const char *xstring::set(const char *s)
{
   return nset(s,xstrlen(s));
}

const char *xstring::set_allocated(char *s)
{
   if(!s)
      return set(0);
   len=strlen(s);
   size=len+1;
   xfree(buf);
   return buf=s;
}

const char *xstring::append(const char *s)
{
   if(!s || !*s)
      return buf;
   if(!buf)
      return set(s);
   size_t s_len=strlen(s);
   get_space(len+s_len);
   memcpy(buf+len,s,s_len);
   len+=s_len;
   return buf;
}

const char *xstring::append(char c)
{
   get_space(len+1);
   buf[len++]=c;
   return buf;
}

static size_t vstrlen(va_list va0)
{
   va_list va;
   VA_COPY(va,va0);
   size_t len=0;
   for(;;)
   {
      const char *s=va_arg(va,const char *);
      if(!s)
	 break;
      len+=strlen(s);
   }
   va_end(va);
   return len;
}
static void vstrcpy(char *buf,va_list va0)
{
   va_list va;
   VA_COPY(va,va0);
   for(;;)
   {
      const char *s=va_arg(va,const char *);
      if(!s)
	 break;
      size_t s_len=strlen(s);
      memcpy(buf,s,s_len);
      buf+=s_len;
   }
   *buf=0;
   va_end(va);
}

const char *xstring::vappend(va_list va)
{
   size_t va_len=vstrlen(va);
   get_space(len+va_len);
   vstrcpy(buf+len,va);
   len+=va_len;
   return buf;
}

const char *xstring::vappend(...)
{
   va_list va;
   va_start(va,this);
   vappend(va);
   va_end(va);
   return buf;
}

const char *xstring::vset(...)
{
   truncate(0);
   va_list va;
   va_start(va,this);
   vappend(va);
   va_end(va);
   return buf;
}

void xstring::truncate(size_t n)
{
   if(n<len)
      set_length(n);
}
void xstring::truncate_at(char c)
{
   if(!buf)
      return;
   char *p=(char*)memchr(buf,c,len);
   if(p)
   {
      *p=0;
      len=p-buf;
   }
}

const char *xstring::set_substr(int start,size_t sublen,const char *s)
{
   size_t s_len=xstrlen(s);
   if(sublen<s_len)
      get_space(len+s_len-sublen);
   if(sublen!=s_len)
      memmove(buf+start+s_len,buf+start+sublen,len-(start+sublen)+1);
   memcpy(buf+start,s,s_len);
   len+=s_len-sublen;
   return buf;
}

bool xstring::chomp(char c)
{
   if(!len || buf[len-1]!=c)
      return false;
   buf[--len]=0;
   return true;
}
void xstring::rtrim(char c)
{
   while(chomp(c));
}

const char *xstring::vsetf(const char *format, va_list ap)
{
   if(size<32 || size>512)
      get_space(strlen(format)+32);
   for(;;)
   {
      va_list tmp;
      VA_COPY(tmp,ap);
      size_t res=vsnprintf(buf, size, format, tmp);
      va_end(tmp);
      if(res>=0 && res<size)
      {
	 set_length(res);
	 return buf;
      }
      get_space(res>size ? res+1 : size*2);
   }
}
const char *xstring::setf(const char *format, ...)
{
   va_list va;
   va_start(va, format);
   vsetf(format, va);
   va_end(va);
   return buf;
}

const char *xstring_c::vset(...)
{
   va_list va;
   va_start(va,this);
   size_t va_len=vstrlen(va);
   if(!buf || strlen(buf)<va_len)
      buf=(char*)xrealloc(buf,va_len+1);
   vstrcpy(buf,va);
   va_end(va);
   return buf;
}