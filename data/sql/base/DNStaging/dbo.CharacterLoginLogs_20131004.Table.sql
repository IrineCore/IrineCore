USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterLoginLogs_20131004]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterLoginLogs_20131004](
	[LoginDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LoginIP] [int] NOT NULL,
	[PCRoomFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CharacterLoginLogs_20131004_LoginDate]    Script Date: 11/14/2026 9:00:06 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CharacterLoginLogs_20131004_LoginDate] ON [dbo].[CharacterLoginLogs_20131004]
(
	[LoginDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CharacterLoginLogs_20131004_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterLoginLogs_20131004_CharacterID] ON [dbo].[CharacterLoginLogs_20131004]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CharacterLoginLogs_20131004]  WITH CHECK ADD  CONSTRAINT [CK_CharacterLoginLogs_20131004_LoginDate] CHECK  (([LoginDate]>='2013-10-04' AND [LoginDate]<'2013-10-05'))
GO
ALTER TABLE [dbo].[CharacterLoginLogs_20131004] CHECK CONSTRAINT [CK_CharacterLoginLogs_20131004_LoginDate]
GO
