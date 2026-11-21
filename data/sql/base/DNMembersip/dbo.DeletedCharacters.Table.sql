USE [dnmembership]
GO
/****** Object:  Table [dbo].[DeletedCharacters]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DeletedCharacters]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[DeletedCharacters](
	[CharacterID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[DeleteDate] [datetime2](0) NOT NULL,
	[DeletePrivateIP] [int] NULL,
	[DeletePublicIP] [int] NOT NULL,
 CONSTRAINT [PK_DeletedCharacters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_DeletedCharacters_CharacterName]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[DeletedCharacters]') AND name = N'IX_NN_DeletedCharacters_CharacterName')
CREATE NONCLUSTERED INDEX [IX_NN_DeletedCharacters_CharacterName] ON [dbo].[DeletedCharacters]
(
	[CharacterName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_DeletedCharacters_DeleteDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[DeletedCharacters]') AND name = N'IX_NN_DeletedCharacters_DeleteDate')
CREATE NONCLUSTERED INDEX [IX_NN_DeletedCharacters_DeleteDate] ON [dbo].[DeletedCharacters]
(
	[DeleteDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_DeletedCharacters_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[DeletedCharacters]'))
ALTER TABLE [dbo].[DeletedCharacters]  WITH CHECK ADD  CONSTRAINT [FK_Characters_DeletedCharacters_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_DeletedCharacters_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[DeletedCharacters]'))
ALTER TABLE [dbo].[DeletedCharacters] CHECK CONSTRAINT [FK_Characters_DeletedCharacters_CharacterID]
GO
