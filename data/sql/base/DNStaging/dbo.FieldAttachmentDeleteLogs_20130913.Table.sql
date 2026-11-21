USE [dnstaging]
GO
/****** Object:  Table [dbo].[FieldAttachmentDeleteLogs_20130913]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldAttachmentDeleteLogs_20130913](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[Attachments] [xml] NOT NULL,
	[HarvestFlag] [bit] NOT NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_FieldAttachmentDeleteLogs_20130913_LogDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_FieldAttachmentDeleteLogs_20130913_LogDate] ON [dbo].[FieldAttachmentDeleteLogs_20130913]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[FieldAttachmentDeleteLogs_20130913]  WITH CHECK ADD  CONSTRAINT [CK_FieldAttachmentDeleteLogs_20130913_LogDate] CHECK  (([LogDate]>='2013-09-13' AND [LogDate]<'2013-09-14'))
GO
ALTER TABLE [dbo].[FieldAttachmentDeleteLogs_20130913] CHECK CONSTRAINT [CK_FieldAttachmentDeleteLogs_20130913_LogDate]
GO
