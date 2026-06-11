'use client'

import { UserDataProvider } from '@/context/UserData'
import { ReactNode } from 'react'
import { Bounce, ToastContainer } from 'react-toastify'

export default function Providers({children}: {children:ReactNode}) {
    
    return <>

        <UserDataProvider>
            {children}
            <ToastContainer
                position="bottom-right"
                autoClose={5000}
                hideProgressBar={false}
                newestOnTop={false}
                closeOnClick={false}
                rtl={false}
                pauseOnFocusLoss
                draggable
                pauseOnHover
                theme="light"
                transition={Bounce}
                />
        </UserDataProvider>
    </>
}
