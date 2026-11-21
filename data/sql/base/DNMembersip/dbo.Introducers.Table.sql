USE [dnmembership]
GO
/****** Object:  Table [dbo].[Introducers]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Introducers]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Introducers](
	[AccountID] [int] NOT NULL,
	[IntroducedAccountID] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_Introducers] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC,
	[IntroducedAccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_Introducers_IntroducedAccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Introducers]') AND name = N'IX_NN_Introducers_IntroducedAccountID')
CREATE NONCLUSTERED INDEX [IX_NN_Introducers_IntroducedAccountID] ON [dbo].[Introducers]
(
	[IntroducedAccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Introducers_RegisterDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Introducers]') AND name = N'IX_NN_Introducers_RegisterDate')
CREATE NONCLUSTERED INDEX [IX_NN_Introducers_RegisterDate] ON [dbo].[Introducers]
(
	[RegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Introducers_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Introducers]'))
ALTER TABLE [dbo].[Introducers]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_Introducers_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Introducers_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Introducers]'))
ALTER TABLE [dbo].[Introducers] CHECK CONSTRAINT [FK_Accounts_Introducers_AccountID]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Introducers_IntroducedAccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Introducers]'))
ALTER TABLE [dbo].[Introducers]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_Introducers_IntroducedAccountID] FOREIGN KEY([IntroducedAccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Introducers_IntroducedAccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Introducers]'))
ALTER TABLE [dbo].[Introducers] CHECK CONSTRAINT [FK_Accounts_Introducers_IntroducedAccountID]
GO
