/***************************************************************************
 *   Copyright (C) 2008 by Mathias Paulin   *
 *   Mathias.Paulin@irit.fr   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef GL_ASSERT_H
#define GL_ASSERT_H

/**
        @author Mathias Paulin <Mathias.Paulin@irit.fr>
 *
 * OpenGL error management class.
 *
 */

//#undef NDEBUG
#ifndef NDEBUG


#include <iostream>
#include <cassert>

#ifdef __APPLE__
#define glAssert(code) \
    code; \
    {\
        GLuint err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            std::cerr<<"erreur OpenGL ("<<__FILE__<<":"<<__LINE__<<", "<<__STRING(code)<<\
                ( (err == GL_INVALID_ENUM) ? " Invalid enum" : \
                ( (err == GL_INVALID_VALUE) ? " Invalid value" : \
                ( (err == GL_INVALID_OPERATION) ? " Invalid operation" : "unknown error") \
                ) \
                )\
                << "("<<err<<")"<<std::endl; \
            /*assert(false);*/ \
        } \
    }

#define glCheckError() \
    {\
        GLuint err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            std::cerr<<"erreur OpenGL ("<<__FILE__<<":"<<__LINE__<<\
    ( (err == GL_INVALID_ENUM) ? " Invalid enum" : \
    ( (err == GL_INVALID_VALUE) ? " Invalid value" : \
    ( (err == GL_INVALID_OPERATION) ? " Invalid operation" : "unknown error") \
    ) \
    )\
    << "("<<err<<")"<<std::endl; \
            /*assert(false);*/ \
        } \
    }
#else
#define glAssert(code) \
    code; \
{\
    GLuint err = glGetError(); \
    if (err != GL_NO_ERROR) { \
    std::cerr<<"erreur OpenGL ("<<__FILE__<<":"<<__LINE__<<", "<<__STRING(code)<<") : "<<(const char*)gluErrorString (err)<<"("<<err<<")"<<std::endl; \
    } \
    }

#define glCheckError() \
    {\
        GLuint err = glGetError(); \
        if (err != GL_NO_ERROR) { \
    std::cerr<<"erreur OpenGL ("<<__FILE__<<":"<<__LINE__<<", "<<__STRING()<<") : "<<(const char*)gluErrorString (err)<<"("<<err<<")"<<std::endl; \
          } \
    }
#endif

#else
#define glAssert(code) \
    code;

#define glCheckError()
#endif

// #undef NDEBUG

#endif
