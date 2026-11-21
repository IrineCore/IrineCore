USE [dnmembership]
GO
/****** Object:  Table [dbo].[CharacterDeleteReviveLogs]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CharacterDeleteReviveLogs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CharacterDeleteReviveLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
	[PrivateIP] [int] NULL,
	[PublicIP] [int] NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_CharacterDeleteReviveLogs_LogDate]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CharacterDeleteReviveLogs]') AND name = N'IX_NC_CharacterDeleteReviveLogs_LogDate')
CREATE CLUSTERED INDEX [IX_NC_CharacterDeleteReviveLogs_LogDate] ON [dbo].[CharacterDeleteReviveLogs]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_CharacterDeleteReviveLogs_CharacterID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CharacterDeleteReviveLogs]') AND name = N'IX_NN_CharacterDeleteReviveLogs_CharacterID')
CREATE NONCLUSTERED INDEX [IX_NN_CharacterDeleteReviveLogs_CharacterID] ON [dbo].[CharacterDeleteReviveLogs]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_CharacterDeleteReviveLogs_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CharacterDeleteReviveLogs]'))
ALTER TABLE [dbo].[CharacterDeleteReviveLogs]  WITH CHECK ADD  CONSTRAINT [FK_Characters_CharacterDeleteReviveLogs_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_CharacterDeleteReviveLogs_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CharacterDeleteReviveLogs]'))
ALTER TABLE [dbo].[CharacterDeleteReviveLogs] CHECK CONSTRAINT [FK_Characters_CharacterDeleteReviveLogs_CharacterID]
GO
