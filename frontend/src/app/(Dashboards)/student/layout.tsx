import type { ReactNode } from "react"
import { Header } from "./_Components/header"
import { Sidebar } from "./_Components/sidebar"
import { GetProfile } from "@/ServerActions/Profile/GetProfile";
import { redirect } from "next/navigation";


export default async function DashboardLayout({ children }: { children: ReactNode }) {
  let profile = null;
  
    try {
      profile = await GetProfile();
    } catch (error) {
      console.error("Not authenticated or profile fetch failed");
    }
    if (!profile) {
      redirect("/login");
    }
  return (
  
        <div className="flex min-h-screen bg-background">
              <Sidebar profile={profile} />
              <div className="flex flex-1 flex-col">
                <Header />
                <main className="flex-1 overflow-auto p-4 md:p-6">{children}</main>
              </div>
            </div>
  )
}
