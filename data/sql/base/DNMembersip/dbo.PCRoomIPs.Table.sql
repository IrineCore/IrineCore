USE [dnmembership]
GO
/****** Object:  Table [dbo].[PCRoomIPs]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PCRoomIPs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PCRoomIPs](
	[IP] [int] NOT NULL,
	[PCRoomGrade] [tinyint] NOT NULL,
 CONSTRAINT [PK_PCRoomIPs] PRIMARY KEY CLUSTERED 
(
	[IP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_PCRoomIPs_PCRoomGrade]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[PCRoomIPs] ADD  CONSTRAINT [DF_PCRoomIPs_PCRoomGrade]  DEFAULT ((1)) FOR [PCRoomGrade]
END
GO
