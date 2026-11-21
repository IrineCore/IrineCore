USE [dnstaging]
GO
/****** Object:  Table [dbo].[FieldAttachmentDeleteLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldAttachmentDeleteLogs](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[Attachments] [xml] NOT NULL,
	[HarvestFlag] [bit] NOT NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
