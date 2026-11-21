USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterLogoutLogs_20130913]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterLogoutLogs_20130913](
	[LogoutDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LoginDate] [datetime2](0) NOT NULL,
	[LoginIP] [int] NOT NULL,
	[SessionID] [int] NOT NULL,
	[PCRoomFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CharacterLogoutLogs_20130913_LogoutDate]    Script Date: 11/14/2026 9:00:06 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CharacterLogoutLogs_20130913_LogoutDate] ON [dbo].[CharacterLogoutLogs_20130913]
(
	[LogoutDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CharacterLogoutLogs_20130913_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterLogoutLogs_20130913_CharacterID] ON [dbo].[CharacterLogoutLogs_20130913]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CharacterLogoutLogs_20130913_LoginIP]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterLogoutLogs_20130913_LoginIP] ON [dbo].[CharacterLogoutLogs_20130913]
(
	[LoginIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CharacterLogoutLogs_20130913]  WITH CHECK ADD  CONSTRAINT [CK_CharacterLogoutLogs_20130913_LogoutDate] CHECK  (([LogoutDate]>='2013-09-13' AND [LogoutDate]<'2013-09-14'))
GO
ALTER TABLE [dbo].[CharacterLogoutLogs_20130913] CHECK CONSTRAINT [CK_CharacterLogoutLogs_20130913_LogoutDate]
GO
