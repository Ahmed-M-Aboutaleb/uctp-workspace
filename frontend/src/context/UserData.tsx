// context/UserDataContext.tsx
'use client'

import { getToken } from "@/cookies/auth.actions"
import { createContext, useContext, useEffect, useState } from "react"

type UserData = {
  token: string | undefined
  role: string | undefined
  email: string | undefined
  profile: string | undefined
}

type UserDataContextType = {
  user: UserData
  setUser: (data: Partial<UserData>) => void
  clearUser: () => void
}

const defaultUser: UserData = {
  token: undefined,
  role: undefined,
  email: undefined,
  profile: undefined,
}

const UserDataContext = createContext<UserDataContextType | undefined>(undefined)

export function UserDataProvider({ children }: { children: React.ReactNode }) {
  const [user, setUserState] = useState<UserData>(defaultUser)

  useEffect(() => {
    const init = async () => {
      const token = await getToken()
      setUserState({
        token,
        role:    localStorage.getItem("role")    ?? undefined,
        email:   localStorage.getItem("email")   ?? undefined,
        profile: localStorage.getItem("profile") ?? undefined,
      })
    }
    init()
  }, [])

  const setUser = (data: Partial<UserData>) => {
    setUserState(prev => ({ ...prev, ...data }))
  }

  const clearUser = () => {
    setUserState(defaultUser)
  }

  return (
    <UserDataContext.Provider value={{ user, setUser, clearUser }}>
      {children}
    </UserDataContext.Provider>
  )
}

export function useUserData() {
  const context = useContext(UserDataContext)
  if (!context) {
    throw new Error("useUserData must be used within a UserDataProvider")
  }
  return context
}