USE [dngsm]
GO
/****** Object:  Table [dbo].[UsedMenuByRole]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[UsedMenuByRole](
	[RoleID] [int] NOT NULL,
	[MenuID] [int] NOT NULL,
	[MenuPriority] [int] NOT NULL,
	[Read] [bit] NOT NULL,
	[Write] [bit] NOT NULL,
 CONSTRAINT [PK_UsedMenuByRole] PRIMARY KEY CLUSTERED 
(
	[MenuID] ASC,
	[RoleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_UsedMenuByRole_MenuID]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_UsedMenuByRole_MenuID] ON [dbo].[UsedMenuByRole]
(
	[MenuID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[UsedMenuByRole] ADD  CONSTRAINT [DF_UsedMenuByRole_Read]  DEFAULT ((0)) FOR [Read]
GO
ALTER TABLE [dbo].[UsedMenuByRole] ADD  CONSTRAINT [DF_UsedMenuByRole_Write]  DEFAULT ((0)) FOR [Write]
GO
ALTER TABLE [dbo].[UsedMenuByRole]  WITH CHECK ADD  CONSTRAINT [FK_Menus_UsedMenuByRole_MenuID] FOREIGN KEY([MenuID])
REFERENCES [dbo].[Menus] ([MenuID])
GO
ALTER TABLE [dbo].[UsedMenuByRole] CHECK CONSTRAINT [FK_Menus_UsedMenuByRole_MenuID]
GO
ALTER TABLE [dbo].[UsedMenuByRole]  WITH CHECK ADD  CONSTRAINT [FK_Roles_UsedMenuByRole_RoleID] FOREIGN KEY([RoleID])
REFERENCES [dbo].[Roles] ([RoleID])
GO
ALTER TABLE [dbo].[UsedMenuByRole] CHECK CONSTRAINT [FK_Roles_UsedMenuByRole_RoleID]
GO
