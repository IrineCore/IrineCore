USE [dnstaging]
GO
/****** Object:  Table [dbo].[PCRoomCharacterLoginLogs_20130917]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PCRoomCharacterLoginLogs_20130917](
	[LoginDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LoginIP] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PCRoomCharacterLoginLogs_20130917_LoginDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PCRoomCharacterLoginLogs_20130917_LoginDate] ON [dbo].[PCRoomCharacterLoginLogs_20130917]
(
	[LoginDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PCRoomCharacterLoginLogs_20130917_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PCRoomCharacterLoginLogs_20130917_CharacterID] ON [dbo].[PCRoomCharacterLoginLogs_20130917]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[PCRoomCharacterLoginLogs_20130917]  WITH CHECK ADD  CONSTRAINT [CK_PCRoomCharacterLoginLogs_20130917_LoginDate] CHECK  (([LoginDate]>='2013-09-17' AND [LoginDate]<'2013-09-18'))
GO
ALTER TABLE [dbo].[PCRoomCharacterLoginLogs_20130917] CHECK CONSTRAINT [CK_PCRoomCharacterLoginLogs_20130917_LoginDate]
GO
