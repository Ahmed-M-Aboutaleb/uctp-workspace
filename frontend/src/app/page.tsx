'use client';
import { useUserData } from "@/context/UserData";
import { redirect } from "next/navigation";

export default function Home() {
  const { role } = useUserData().user;
  const {token} = useUserData().user;
  if (role === "admin"){
    redirect("/admin");
  }else if (role === "instructor"){
    redirect("/instructor")
  }else if (role === "student"){
    redirect("/student")
  }

  if(!token){
    redirect("/login")
  }
}
