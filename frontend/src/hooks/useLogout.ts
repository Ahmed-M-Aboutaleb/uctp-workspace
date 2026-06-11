// hooks/useLogout.ts
import { useUserData } from "@/context/UserData"
import { deleteToken } from "@/cookies/auth.actions"
import { useRouter } from "next/navigation"
import { useState } from "react"
import { toast } from "react-toastify"

export function useLogout() {
  const [isLoggingOut, setIsLoggingOut] = useState(false)
  const { clearUser } = useUserData()
  const router = useRouter()

  const logout = async () => {
    setIsLoggingOut(true)
    try {
      await deleteToken()
      localStorage.clear()
      clearUser();
      toast.success("Logged out successfully")
      router.push("/login")
    } catch {
      toast.error("Failed to logout, please try again")
    } finally {
      setIsLoggingOut(false)
    }
  }

  return { logout, isLoggingOut }
}