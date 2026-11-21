USE [dnmembership]
GO
/****** Object:  Table [dbo].[Restraints]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Restraints]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Restraints](
	[RestraintID] [int] IDENTITY(1,1) NOT NULL,
	[RestraintTargetCode] [tinyint] NOT NULL,
	[RestraintTypeCode] [tinyint] NOT NULL,
	[AccountID] [int] NULL,
	[CharacterID] [bigint] NULL,
	[AdminID] [int] NOT NULL,
	[RestraintReasonID] [int] NOT NULL,
	[Memo] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NULL,
	[RestraintMessage] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NULL,
	[StartDate] [smalldatetime] NOT NULL,
	[EndDate] [smalldatetime] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[CancelFlag] [bit] NOT NULL,
 CONSTRAINT [PK_Restraints] PRIMARY KEY CLUSTERED 
(
	[RestraintID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_Restraints_AdminID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Restraints]') AND name = N'IX_NN_Restraints_AdminID')
CREATE NONCLUSTERED INDEX [IX_NN_Restraints_AdminID] ON [dbo].[Restraints]
(
	[AdminID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Restraints_RegisterDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Restraints]') AND name = N'IX_NN_Restraints_RegisterDate')
CREATE NONCLUSTERED INDEX [IX_NN_Restraints_RegisterDate] ON [dbo].[Restraints]
(
	[RegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Restraints_RestraintReasonID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Restraints]') AND name = N'IX_NN_Restraints_RestraintReasonID')
CREATE NONCLUSTERED INDEX [IX_NN_Restraints_RestraintReasonID] ON [dbo].[Restraints]
(
	[RestraintReasonID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Restraints_AccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Restraints]') AND name = N'IX_NNI_Restraints_AccountID')
CREATE NONCLUSTERED INDEX [IX_NNI_Restraints_AccountID] ON [dbo].[Restraints]
(
	[AccountID] ASC
)
INCLUDE([StartDate],[EndDate],[CancelFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Restraints_CharacterID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Restraints]') AND name = N'IX_NNI_Restraints_CharacterID')
CREATE NONCLUSTERED INDEX [IX_NNI_Restraints_CharacterID] ON [dbo].[Restraints]
(
	[CharacterID] ASC
)
INCLUDE([StartDate],[EndDate],[CancelFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Restraints_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Restraints]'))
ALTER TABLE [dbo].[Restraints]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_Restraints_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_Restraints_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Restraints]'))
ALTER TABLE [dbo].[Restraints] CHECK CONSTRAINT [FK_Accounts_Restraints_AccountID]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_Restraints_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Restraints]'))
ALTER TABLE [dbo].[Restraints]  WITH CHECK ADD  CONSTRAINT [FK_Characters_Restraints_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_Restraints_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[Restraints]'))
ALTER TABLE [dbo].[Restraints] CHECK CONSTRAINT [FK_Characters_Restraints_CharacterID]
GO
