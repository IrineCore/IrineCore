USE [dnstaging]
GO
/****** Object:  Table [dbo].[PCRoomCharacterLogoutLogs_20131004]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PCRoomCharacterLogoutLogs_20131004](
	[LogoutDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LoginDate] [datetime2](0) NOT NULL,
	[LoginIP] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PCRoomCharacterLogoutLogs_20131004_LogoutDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PCRoomCharacterLogoutLogs_20131004_LogoutDate] ON [dbo].[PCRoomCharacterLogoutLogs_20131004]
(
	[LogoutDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PCRoomCharacterLogoutLogs_20131004_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PCRoomCharacterLogoutLogs_20131004_CharacterID] ON [dbo].[PCRoomCharacterLogoutLogs_20131004]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PCRoomCharacterLogoutLogs_20131004_LoginIP]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PCRoomCharacterLogoutLogs_20131004_LoginIP] ON [dbo].[PCRoomCharacterLogoutLogs_20131004]
(
	[LoginIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[PCRoomCharacterLogoutLogs_20131004]  WITH CHECK ADD  CONSTRAINT [CK_PCRoomCharacterLogoutLogs_20131004_LogoutDate] CHECK  (([LogoutDate]>='2013-10-04' AND [LogoutDate]<'2013-10-05'))
GO
ALTER TABLE [dbo].[PCRoomCharacterLogoutLogs_20131004] CHECK CONSTRAINT [CK_PCRoomCharacterLogoutLogs_20131004_LogoutDate]
GO
