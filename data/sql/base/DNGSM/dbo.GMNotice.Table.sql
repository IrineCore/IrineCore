USE [dngsm]
GO
/****** Object:  Table [dbo].[GMNotice]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GMNotice](
	[LogDate] [datetime] NULL,
	[NoticeIndex] [int] NULL,
	[NoticeType] [int] NULL,
	[NoticeIsSlide] [bit] NULL,
	[NoticeServer] [nvarchar](1000) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NoticeChannel] [nvarchar](1000) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NoticeMapindex] [nvarchar](1000) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NoticeStartDate] [datetime] NULL,
	[NoticeEndDate] [datetime] NULL,
	[NoticeDelay] [int] NULL,
	[NoticeContents] [nvarchar](120) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UserID] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
