USE [dnmembership]
GO
/****** Object:  Table [dbo].[Characters]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Characters]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Characters](
	[CharacterID] [bigint] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NULL,
	[WorldID] [int] NOT NULL,
	[CreateDate] [datetime2](0) NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
	[DeleteByAdminFlag] [bit] NULL,
 CONSTRAINT [PK_Characters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_Characters_AccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Characters]') AND name = N'IX_NN_Characters_AccountID')
CREATE NONCLUSTERED INDEX [IX_NN_Characters_AccountID] ON [dbo].[Characters]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Characters_CreateDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Characters]') AND name = N'IX_NN_Characters_CreateDate')
CREATE NONCLUSTERED INDEX [IX_NN_Characters_CreateDate] ON [dbo].[Characters]
(
	[CreateDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UNF_Characters_CharacterName]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Characters]') AND name = N'IX_UNF_Characters_CharacterName')
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_Characters_CharacterName] ON [dbo].[Characters]
(
	[CharacterName] ASC
)
WHERE ([CharacterName] IS NOT NULL)
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Characters_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Characters]'))
ALTER TABLE [dbo].[Characters]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_Characters_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Characters_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Characters]'))
ALTER TABLE [dbo].[Characters] CHECK CONSTRAINT [FK_Accounts_Characters_AccountID]
GO
