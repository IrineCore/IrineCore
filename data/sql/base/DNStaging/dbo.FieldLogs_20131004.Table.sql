USE [dnstaging]
GO
/****** Object:  Table [dbo].[FieldLogs_20131004]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldLogs_20131004](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ElapsedTime] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_FieldLogs_20131004_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_FieldLogs_20131004_LogDate] ON [dbo].[FieldLogs_20131004]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[FieldLogs_20131004]  WITH CHECK ADD  CONSTRAINT [CK_FieldLogs_20131004_LogDate] CHECK  (([LogDate]>='2013-10-04' AND [LogDate]<'2013-10-05'))
GO
ALTER TABLE [dbo].[FieldLogs_20131004] CHECK CONSTRAINT [CK_FieldLogs_20131004_LogDate]
GO
