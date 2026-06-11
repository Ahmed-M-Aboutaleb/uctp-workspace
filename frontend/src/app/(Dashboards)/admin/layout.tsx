import { GetProfile } from "@/ServerActions/Profile/GetProfile";
import { redirect } from "next/navigation";
import { AdminHeader } from "./_Components/admin-header";
import { AdminSidebar } from "./_Components/admin-sidebar";


export default async function AdminLayout({
  children,
}: {
  children: React.ReactNode
}) {
  let profile = null;

  try {
    profile = await GetProfile();
  } catch (error) {
    console.error("Not authenticated or profile fetch failed");
  }
  if (!profile) {
    redirect("/login");
  }
    profile.full_name = profile.full_name.toUpperCase();


  return (
    <div className="flex min-h-screen bg-background">
      <AdminSidebar profile={profile} />
      <div className="flex flex-1 flex-col">
        <AdminHeader />
        <main className="flex-1 overflow-auto p-4 md:p-6">{children}</main>
      </div>
    </div>
  )
}
