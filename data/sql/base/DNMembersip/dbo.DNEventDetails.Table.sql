USE [dnmembership]
GO
/****** Object:  Table [dbo].[DNEventDetails]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DNEventDetails]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[DNEventDetails](
	[DNEventDetailID] [int] IDENTITY(1,1) NOT NULL,
	[EventName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[EventType1] [tinyint] NOT NULL,
	[EventType2] [tinyint] NULL,
	[EventType3] [tinyint] NULL,
	[EventAtt1] [int] NOT NULL,
	[EventAtt2] [int] NULL,
	[EventAtt3] [int] NULL,
	[EventAtt4] [int] NULL,
	[EventAtt5] [int] NULL,
	[DeleteFlag] [bit] NOT NULL,
	[MapIDs] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_DNEventDetails] PRIMARY KEY CLUSTERED 
(
	[DNEventDetailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNEventDetails_DeleteFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNEventDetails] ADD  CONSTRAINT [DF_DNEventDetails_DeleteFlag]  DEFAULT ((0)) FOR [DeleteFlag]
END
GO
