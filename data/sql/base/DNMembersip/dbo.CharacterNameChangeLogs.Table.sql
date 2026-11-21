USE [dnmembership]
GO
/****** Object:  Table [dbo].[CharacterNameChangeLogs]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CharacterNameChangeLogs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CharacterNameChangeLogs](
	[CharacterNameChangeLogID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterNameBefore] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterNameAfter] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterNameChangeCode] [tinyint] NOT NULL,
	[ChangeDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_CharacterNameChangeLogs] PRIMARY KEY CLUSTERED 
(
	[CharacterNameChangeLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_CharacterNameChangeLogs_CharacterID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CharacterNameChangeLogs]') AND name = N'IX_NN_CharacterNameChangeLogs_CharacterID')
CREATE NONCLUSTERED INDEX [IX_NN_CharacterNameChangeLogs_CharacterID] ON [dbo].[CharacterNameChangeLogs]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NNI_CharacterNameChangeLogs_CharacterNameBefore]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CharacterNameChangeLogs]') AND name = N'IX_NNI_CharacterNameChangeLogs_CharacterNameBefore')
CREATE NONCLUSTERED INDEX [IX_NNI_CharacterNameChangeLogs_CharacterNameBefore] ON [dbo].[CharacterNameChangeLogs]
(
	[CharacterNameBefore] ASC
)
INCLUDE([ChangeDate]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_CharacterNameChangeLogs_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CharacterNameChangeLogs]'))
ALTER TABLE [dbo].[CharacterNameChangeLogs]  WITH CHECK ADD  CONSTRAINT [FK_Characters_CharacterNameChangeLogs_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_CharacterNameChangeLogs_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CharacterNameChangeLogs]'))
ALTER TABLE [dbo].[CharacterNameChangeLogs] CHECK CONSTRAINT [FK_Characters_CharacterNameChangeLogs_CharacterID]
GO
