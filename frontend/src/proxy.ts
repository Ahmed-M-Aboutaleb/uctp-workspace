import { NextRequest, NextResponse } from "next/server";

const protectedRoutes = [
    '/admin',
    '/student',
    '/instructor',
];

const AuthRoutes = [
    '/login',
]

export function proxy(request:NextRequest){
    const {pathname}=request.nextUrl;
    const token = request.cookies.get('token')?.value;

    const isAuthenticated = !!token;

    const isProtectedRoute = protectedRoutes.some(
        (route)=> route == pathname || pathname.startsWith(`${route}/`)
        );

    const isAuthRoute = AuthRoutes.some(
        (route)=> route == pathname || pathname.startsWith(`${route}/`)
        );

        if(isProtectedRoute && !isAuthenticated){
            return NextResponse.redirect(new URL('/login',request.url))
        }
        if(isAuthRoute && isAuthenticated){
            return NextResponse.redirect(new URL('/',request.url))
        }


        return NextResponse.next();

}


export const config = {
    matcher:[
    '/login',
    '/admin',
    '/student',
    '/instructor'
    ]
}